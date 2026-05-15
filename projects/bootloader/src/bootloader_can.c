/************************************************************************************************
 * @file   bootloader_can.c
 *
 * @brief  Source file for CAN communication in the bootloader
 *
 * @date   2025-01-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#include "stm32l4xx.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_can.h"
#include "stm32l4xx_hal_rcc.h"
#include "stm32l4xx_hal_gpio.h"
#include "gpio.h"

/* Intra-component Headers */
#include "bootloader_can.h"

/* CAN has 3 transmit mailboxes and 2 receive FIFOs */
#define CAN_HW_BASE       CAN1
#define MAX_TX_RETRIES    3
#define MAX_TX_MS_TIMEOUT 20

extern uint32_t HAL_GetTick(void);

typedef enum {
  CAN_HW_BUS_STATUS_OK = 0,
  CAN_HW_BUS_STATUS_ERROR,
  CAN_HW_BUS_STATUS_OFF,
} CanHwBusStatus;

static CAN_HandleTypeDef s_can_handle;

static BootCanTiming s_timing[NUM_BOOT_CAN_BITRATES] = {
  [BOOT_CAN_BITRATE_125KBPS] = { .prescaler = 40, .bs1 = CAN_BS1_13TQ, .bs2 = CAN_BS2_2TQ },
  [BOOT_CAN_BITRATE_250KBPS] = { .prescaler = 20, .bs1 = CAN_BS1_13TQ, .bs2 = CAN_BS2_2TQ },
  [BOOT_CAN_BITRATE_500KBPS] = { .prescaler = 10, .bs1 = CAN_BS1_11TQ, .bs2 = CAN_BS2_4TQ },
  [BOOT_CAN_BITRATE_1000KBPS] = { .prescaler = 5, .bs1 = CAN_BS1_13TQ, .bs2 = CAN_BS2_2TQ }
};

static BootloaderError s_can_gpio_init(void) {
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitTypeDef gpio_init = {
    .Pin = GPIO_PIN_8 | GPIO_PIN_9,
    .Mode = GPIO_MODE_AF_PP,
    .Pull = GPIO_PULLUP,
    .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
    .Alternate = GPIO_AF9_CAN1,
  };
  HAL_GPIO_Init(GPIOB, &gpio_init);

  return BOOTLOADER_ERROR_NONE;
}

static CanHwBusStatus s_bus_status(void) {
  uint32_t error_flags = HAL_CAN_GetError(&s_can_handle);

  if (error_flags & HAL_CAN_ERROR_BOF) {
    return CAN_HW_BUS_STATUS_OFF;
  } else if (error_flags & (HAL_CAN_ERROR_EWG | HAL_CAN_ERROR_EPV)) {
    return CAN_HW_BUS_STATUS_ERROR;
  }

  return CAN_HW_BUS_STATUS_OK;
}

BootloaderError boot_can_init(const Boot_CanSettings *settings) {
  if (settings == NULL) {
    return BOOTLOADER_INVALID_ARGS;
  }

  if (s_can_gpio_init() != BOOTLOADER_ERROR_NONE) {
    return BOOTLOADER_CAN_INIT_ERR;
  }

  __HAL_RCC_CAN1_CLK_ENABLE();
  __HAL_RCC_CAN1_FORCE_RESET();
  __HAL_RCC_CAN1_RELEASE_RESET();

  uint32_t can_mode = CAN_MODE_NORMAL;
  if (settings->loopback && settings->silent) {
    can_mode = CAN_MODE_SILENT_LOOPBACK;
  } else if (settings->loopback) {
    can_mode = CAN_MODE_LOOPBACK;
  } else if (settings->silent) {
    can_mode = CAN_MODE_SILENT;
  }

  s_can_handle.Instance = CAN_HW_BASE;
  s_can_handle.Init.Prescaler = s_timing[settings->bitrate].prescaler;
  s_can_handle.Init.Mode = can_mode;
  s_can_handle.Init.SyncJumpWidth = CAN_SJW_1TQ;                  /* Maximum time quantum jumps for resynchronization of bus nodes */
  s_can_handle.Init.TimeSeg1 = s_timing[settings->bitrate].bs1;   /* Time permitted before sample point (Prop + Phase seg) */
  s_can_handle.Init.TimeSeg2 = s_timing[settings->bitrate].bs2;   /* Time permitted after sample point */
  s_can_handle.Init.TimeTriggeredMode = DISABLE;                  /* Traditional CAN behaviour based on priority and arbitration */
  s_can_handle.Init.AutoBusOff = ENABLE;                          /* Auto error handling. Turns bus off when many errors detected */
  s_can_handle.Init.AutoWakeUp = DISABLE;                         /* Node stays in sleep until explicitly woken  */
  s_can_handle.Init.AutoRetransmission = DISABLE;                 /* We use one-shot transmission since auto retransmit can cause timing issues */
  s_can_handle.Init.ReceiveFifoLocked = DISABLE;                  /* Ensures latest data is always available */
  s_can_handle.Init.TransmitFifoPriority = DISABLE;               /* Message priority is driven off ID rather than order in FIFO */

  if (HAL_CAN_Init(&s_can_handle) != HAL_OK) {
    return BOOTLOADER_INTERNAL_ERR;
  }

  CAN_FilterTypeDef filter = {
    .FilterBank = 0,
    .FilterMode = CAN_FILTERMODE_IDMASK,
    .FilterScale = CAN_FILTERSCALE_32BIT,
    .FilterIdHigh = 0x0000,
    .FilterIdLow = 0x0000,
    .FilterMaskIdHigh = 0x0000,
    .FilterMaskIdLow = 0x0000,
    .FilterFIFOAssignment = CAN_RX_FIFO0,
    .FilterActivation = ENABLE,
    .SlaveStartFilterBank = 14,
  };

  if (HAL_CAN_ConfigFilter(&s_can_handle, &filter) != HAL_OK) {
    return BOOTLOADER_INTERNAL_ERR;
  }

  if (HAL_CAN_Start(&s_can_handle) != HAL_OK) {
    return BOOTLOADER_INTERNAL_ERR;
  }

  return BOOTLOADER_ERROR_NONE;
}

BootloaderError boot_can_transmit(uint32_t id, bool extended, const uint8_t *data, size_t len) {
  if (data == NULL || len > 8U) {
    return BOOTLOADER_INVALID_ARGS;
  }

  CAN_TxHeaderTypeDef tx_header = {
    .StdId = id,
    .ExtId = id,
    .IDE = extended ? CAN_ID_EXT : CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = len,
    .TransmitGlobalTime = DISABLE,
  };

  uint32_t tx_mailbox = 0;

  for (size_t i = 0; i < MAX_TX_RETRIES; ++i) {
    if (s_bus_status() == CAN_HW_BUS_STATUS_OFF) {
      return BOOTLOADER_CAN_TRANSMIT_ERROR;
    }

    /* Poll for a free mailbox */
    uint32_t deadline = HAL_GetTick() + MAX_TX_MS_TIMEOUT;
    while (HAL_CAN_GetTxMailboxesFreeLevel(&s_can_handle) == 0U) {
      if (HAL_GetTick() >= deadline) {
        break;
      }
    }

    if (HAL_CAN_GetTxMailboxesFreeLevel(&s_can_handle) == 0U) {
      continue;
    }

    HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(&s_can_handle, &tx_header,
                                                    (uint8_t *)data, &tx_mailbox);
    if (status == HAL_OK) {
      return BOOTLOADER_ERROR_NONE;
    }

    /* Race: mailbox went full between our check and AddTxMessage */
    if (HAL_CAN_GetTxMailboxesFreeLevel(&s_can_handle) == 0U) {
      continue;
    }

    return BOOTLOADER_CAN_TRANSMIT_ERROR;
  }

  return BOOTLOADER_INTERNAL_ERR;
}

BootloaderError boot_can_receive(Boot_CanMessage *const msg) {
  if (msg == NULL) {
    return BOOTLOADER_INVALID_ARGS;
  }

  CAN_RxHeaderTypeDef rx_header;
  uint8_t rx_data[8];

  if (HAL_CAN_GetRxFifoFillLevel(&s_can_handle, CAN_RX_FIFO0) > 0) {
    if (HAL_CAN_GetRxMessage(&s_can_handle, CAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK) {
      msg->extended = (rx_header.IDE == CAN_ID_EXT);
      msg->id = msg->extended ? rx_header.ExtId : rx_header.StdId;
      msg->dlc = rx_header.DLC;

      memcpy(&msg->data, rx_data, rx_header.DLC);
      return BOOTLOADER_ERROR_NONE;
    }
  } else if (HAL_CAN_GetRxFifoFillLevel(&s_can_handle, CAN_RX_FIFO1) > 0) {
    if (HAL_CAN_GetRxMessage(&s_can_handle, CAN_RX_FIFO1, &rx_header, rx_data) == HAL_OK) {
      msg->extended = (rx_header.IDE == CAN_ID_EXT);
      msg->id = msg->extended  ? rx_header.ExtId : rx_header.StdId;
      msg->dlc = rx_header.DLC;
      memcpy(&msg->data, rx_data, rx_header.DLC);
      return BOOTLOADER_ERROR_NONE;
    }
  }

  return BOOTLOADER_CAN_RECEIVE_ERROR;
}
