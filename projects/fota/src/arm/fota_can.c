/************************************************************************************************
 * @file   fota_can.c
 *
 * @brief  Source file for CAN communication in the bootloader
 *
 * @date   2025-01-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#include "gpio.h"
#include "stm32l4xx.h"
#include "stm32l4xx_hal_conf.h"

/* Hardware Headers */
#include "stm32l4xx_hal_can.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_rcc.h"

/* Intra-component Headers */
#include "fota_can.h"

/* CAN has 3 transmit mailboxes and 2 receive FIFOs */
#define CAN_HW_BASE CAN1
#define MAX_TX_RETRIES 3
#define MAX_TX_MS_TIMEOUT 20

static CAN_HandleTypeDef s_can_handle;

static FotaCanTiming s_timing[NUM_FOTA_CAN_BITRATES] = { [FOTA_CAN_BITRATE_125KBPS] = { .prescaler = 40, .bs1 = CAN_BS1_12TQ, .bs2 = CAN_BS2_1TQ },
                                                         [FOTA_CAN_BITRATE_250KBPS] = { .prescaler = 20, .bs1 = CAN_BS1_12TQ, .bs2 = CAN_BS2_1TQ },
                                                         [FOTA_CAN_BITRATE_500KBPS] = { .prescaler = 10, .bs1 = CAN_BS1_12TQ, .bs2 = CAN_BS2_1TQ },
                                                         [FOTA_CAN_BITRATE_1000KBPS] = { .prescaler = 5, .bs1 = CAN_BS1_12TQ, .bs2 = CAN_BS2_1TQ } };

static FotaError s_can_gpio_init() {
  GPIO_InitTypeDef rx_init = { .Pin = 8, .Mode = GPIO_MODE_AF_PP, .Pull = 0x00000000u, .Speed = 0x00000003u, .Alternate = 0x09 };
  GPIO_InitTypeDef tx_init = { .Pin = 9, .Mode = GPIO_MODE_AF_PP, .Pull = 0x00000000u, .Speed = 0x00000003u, .Alternate = 0x09 };

  GPIO_TypeDef *rx_gpio_port = (GPIO_TypeDef *)(AHB2PERIPH_BASE + (GPIO_PORT_B * GPIO_ADDRESS_OFFSET));
  GPIO_TypeDef *tx_gpio_port = (GPIO_TypeDef *)(AHB2PERIPH_BASE + (GPIO_PORT_B * GPIO_ADDRESS_OFFSET));

  HAL_GPIO_Init(rx_gpio_port, &rx_init);
  HAL_GPIO_Init(tx_gpio_port, &tx_init);

  return FOTA_ERROR_SUCCESS;
}

FotaError boot_can_init(const FotaCanSettings *settings) {
  if (settings == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  if (s_can_gpio_init() != FOTA_ERROR_SUCCESS) {
    return FOTA_CAN_INIT_ERR;
  }

  __HAL_RCC_CAN1_CLK_ENABLE();

  uint32_t can_mode = CAN_MODE_NORMAL;
  if (settings->loopback) {
    can_mode |= CAN_MODE_LOOPBACK;
  }
  if (settings->silent) {
    can_mode |= CAN_MODE_SILENT;
  }

  s_can_handle.Instance = CAN_HW_BASE;
  s_can_handle.Init.Prescaler = s_timing[settings->bitrate].prescaler;
  s_can_handle.Init.Mode = can_mode;
  s_can_handle.Init.SyncJumpWidth = CAN_SJW_1TQ;                /* Maximum time quantum jumps for resynchronization of bus nodes */
  s_can_handle.Init.TimeSeg1 = s_timing[settings->bitrate].bs1; /* Time permitted before sample point (Prop + Phase seg) */
  s_can_handle.Init.TimeSeg2 = s_timing[settings->bitrate].bs2; /* Time permitted after sample point */
  s_can_handle.Init.TimeTriggeredMode = DISABLE;                /* Traditional CAN behaviour based on priority and arbitration */
  s_can_handle.Init.AutoBusOff = ENABLE;                        /* Auto error handling. Turns bus off when many errors detected */
  s_can_handle.Init.AutoWakeUp = DISABLE;                       /* Node stays in sleep until explicitly woken  */
  s_can_handle.Init.AutoRetransmission = DISABLE;               /* We use one-shot transmission since auto retransmit can cause timing issues */
  s_can_handle.Init.ReceiveFifoLocked = DISABLE;                /* Ensures latest data is always available */
  s_can_handle.Init.TransmitFifoPriority = DISABLE;             /* Message priority is driven off ID rather than order in FIFO */

  if (HAL_CAN_Init(&s_can_handle) != HAL_OK) {
    return FOTA_CAN_INIT_ERR;
  }

  if (HAL_CAN_Start(&s_can_handle) != HAL_OK) {
    return FOTA_CAN_INIT_ERR;
  }

  return FOTA_ERROR_SUCCESS;
}

FotaError boot_can_transmit(uint32_t id, bool extended, const uint8_t *data, size_t len) {
  if (data == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  if (len > 8U) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  CAN_TxHeaderTypeDef tx_header = { .StdId = id, .ExtId = id, .IDE = extended ? CAN_ID_EXT : CAN_ID_STD, .RTR = CAN_RTR_DATA, .DLC = len, .TransmitGlobalTime = DISABLE };

  uint32_t tx_mailbox;
  HAL_StatusTypeDef status;

  for (size_t i = 0; i < MAX_TX_RETRIES; ++i) {
    status = HAL_CAN_AddTxMessage(&s_can_handle, &tx_header, data, &tx_mailbox);

    if (status == HAL_OK) {
      return FOTA_ERROR_SUCCESS;
    } else if (status == HAL_BUSY) {
      /* For future expansion, delay for MAX_TX_MS_TIMEOUT */
      continue;
    } else {
      return FOTA_CAN_TRANSMISSION_ERROR;
    }
  }

  return FOTA_ERROR_INTERNAL_ERROR;
}

FotaError boot_can_receive(FotaCanMessage *const msg) {
  if (msg == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  CAN_RxHeaderTypeDef rx_header;
  uint8_t rx_data[8];

  if (HAL_CAN_GetRxFifoFillLevel(&s_can_handle, CAN_RX_FIFO0) > 0) {
    if (HAL_CAN_GetRxMessage(&s_can_handle, CAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK) {
      msg->extended = (rx_header.IDE == CAN_ID_EXT);
      msg->id = msg->extended ? rx_header.ExtId : rx_header.StdId;
      msg->dlc = rx_header.DLC;

      memcpy(&msg->data, rx_data, rx_header.DLC);
      return FOTA_ERROR_SUCCESS;
    }
  } else if (HAL_CAN_GetRxFifoFillLevel(&s_can_handle, CAN_RX_FIFO1) > 0) {
    if (HAL_CAN_GetRxMessage(&s_can_handle, CAN_RX_FIFO1, &rx_header, rx_data) == HAL_OK) {
      msg->extended = (rx_header.IDE == CAN_ID_EXT);
      msg->id = msg->extended ? rx_header.ExtId : rx_header.StdId;
      msg->dlc = rx_header.DLC;
      memcpy(&msg->data, rx_data, rx_header.DLC);
      return FOTA_ERROR_SUCCESS;
    }
  }

  return FOTA_CAN_RECEIVE_ERROR;
}
