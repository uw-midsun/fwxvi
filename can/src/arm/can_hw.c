/************************************************************************************************
 * @file   can_hw.c
 *
 * @brief  Source file for CAN HW Interface
 *
 * @date   2024-11-03
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>
#include <string.h>

/* Inter-component Headers */
#include "stm32l433xx.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_rcc.h"
#include "stm32l4xx_hal_can.h"
#include "log.h"
#include "interrupts.h"

/* Intra-component Headers */
#include "can_hw.h"

/* CAN has 3 transmit mailboxes and 2 receive FIFOs */
#define CAN_HW_BASE CAN1
#define MAX_TX_RETRIES            3U
#define MAX_TX_MS_TIMEOUT         20U

/* STM32L4 has 14 filter banks */
#define CAN_HW_NUM_FILTER_BANKS   14U

#define CAN_NUM_MAILBOXES         3U

/*
 * CAN Timing Explanation:
 * - One CAN bit is divided into multiple time quanta (TQ)
 * - Total TQ = Sync_Seg(1) + Prop_Seg + Phase_Seg1 + Phase_Seg2
 * - Sync_Seg: Always 1 TQ, used for synchronization
 * - Prop_Seg + Phase_Seg1 = BS1: Used for sampling
 * - Phase_Seg2 = BS2: Used for processing and accounting for phase errors
 * - Sample point should typically be around 75-80% of the bit time
 *
 * Calculation:
 * TQ = 1/Bitrate * Prescaler
 * Total TQ = 1 + BS1 + BS2 (usually 16-25 TQ total)
 */
typedef struct CanHwTiming {
  uint16_t prescaler;
  uint32_t bs1;
  uint32_t bs2;
} CanHwTiming;

/* Generated settings using http://www.bittiming.can-wiki.info/ */
/* For 80 MHz APB1 clock. CAN Sampling occurs at ~87.5% of the frame */
static CanHwTiming s_timing[NUM_CAN_HW_BITRATES] = {
  [CAN_HW_BITRATE_125KBPS] = { .prescaler = 40, .bs1 = CAN_BS1_12TQ, .bs2 = CAN_BS2_1TQ },
  [CAN_HW_BITRATE_250KBPS] = { .prescaler = 20, .bs1 = CAN_BS1_12TQ, .bs2 = CAN_BS2_1TQ },
  [CAN_HW_BITRATE_500KBPS] = { .prescaler = 8, .bs1 = CAN_BS1_15TQ, .bs2 = CAN_BS2_2TQ },
  [CAN_HW_BITRATE_1000KBPS] = { .prescaler = 5, .bs1 = CAN_BS1_12TQ, .bs2 = CAN_BS2_1TQ }
};

static uint8_t s_num_filters;
static CanQueue *s_g_rx_queue;
static CAN_HandleTypeDef s_can_handle;

static SemaphoreHandle_t s_can_tx_ready_sem_handle;
static StaticSemaphore_t s_can_tx_ready_sem;
static bool s_tx_full = false;

/* 1 for filter in, 2 for filter out, default = 0 */
static int s_can_filter_en = 0;
static uint32_t can_filters[CAN_HW_NUM_FILTER_BANKS];

static void s_add_filter_in(uint8_t filter_num, uint32_t mask, uint32_t filter) {
  CAN_FilterTypeDef filter_cfg = {
    .FilterBank = filter_num,
    .FilterMode = CAN_FILTERMODE_IDMASK,
    .FilterScale = CAN_FILTERSCALE_32BIT,
    .FilterIdHigh = (filter >> 16),
    .FilterIdLow = (uint16_t)filter,
    .FilterMaskIdHigh = (mask >> 16),
    .FilterMaskIdLow = (uint16_t)mask,
    .FilterFIFOAssignment = (filter_num % 2) ? CAN_RX_FIFO1 : CAN_RX_FIFO0,
    .FilterActivation = ENABLE,
    .SlaveStartFilterBank = 14
  };

  HAL_CAN_ConfigFilter(&s_can_handle, &filter_cfg);
}

StatusCode can_hw_init(const CanQueue *rx_queue, const CanSettings *settings) {
  if (rx_queue == NULL || settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  
  gpio_init_pin_af(&settings->tx, GPIO_ALTFN_OPEN_DRAIN, GPIO_ALT9_CAN1);
  gpio_init_pin_af(&settings->rx, GPIO_ALTFN_OPEN_DRAIN, GPIO_ALT9_CAN1);

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
    return STATUS_CODE_INTERNAL_ERROR;
  }

  /* Allow all messages by default, but reset the filter count so it's overwritten on the first filter */
  s_add_filter_in(0, 0, 0);
  s_num_filters = 0;

  if (HAL_CAN_Start(&s_can_handle) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  /* Enable all interrupts */
  interrupt_nvic_enable(CAN1_TX_IRQn, INTERRUPT_PRIORITY_HIGH);
  interrupt_nvic_enable(CAN1_RX0_IRQn, INTERRUPT_PRIORITY_HIGH);
  interrupt_nvic_enable(CAN1_RX1_IRQn, INTERRUPT_PRIORITY_HIGH);
  interrupt_nvic_enable(CAN1_SCE_IRQn, INTERRUPT_PRIORITY_HIGH);

  HAL_CAN_ActivateNotification(&s_can_handle, CAN_IT_TX_MAILBOX_EMPTY |
                                          CAN_IT_RX_FIFO0_MSG_PENDING |
                                          CAN_IT_RX_FIFO1_MSG_PENDING |
                                          CAN_IT_ERROR);

  /* Initialize CAN queue */
  s_g_rx_queue = rx_queue;

  /* Create available mailbox semaphore */
  s_can_tx_ready_sem_handle = xSemaphoreCreateCountingStatic(CAN_NUM_MAILBOXES, CAN_NUM_MAILBOXES, &s_can_tx_ready_sem);
  configASSERT(s_can_tx_ready_sem_handle);
  s_tx_full = false;

  LOG_DEBUG("CAN HW initialized on %s\n", CAN_HW_DEV_INTERFACE);

  return STATUS_CODE_OK;
}

StatusCode can_hw_add_filter_in(uint32_t mask, uint32_t filter, bool extended) {
  if (s_can_filter_en == 0) {
    s_can_filter_en = 1;
  }

  if (s_num_filters >= CAN_HW_NUM_FILTER_BANKS) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  } else if (s_can_filter_en != 1) {
    return STATUS_CODE_UNINITIALIZED;
  }

  size_t offset = extended ? 3 : 21;
  uint32_t mask_val = (mask << offset) | (1 << 2);
  uint32_t filter_val = (filter << offset) | ((uint32_t)extended << 2);

  s_add_filter_in(s_num_filters, mask_val, filter_val);
  s_num_filters++;
  return STATUS_CODE_OK;
}

CanHwBusStatus can_hw_bus_status(void) {
  uint32_t error_flags = HAL_CAN_GetError(&s_can_handle);
  
  if (error_flags & HAL_CAN_ERROR_BOF) {
    return CAN_HW_BUS_STATUS_OFF;
  } else if (error_flags & (HAL_CAN_ERROR_EWG | HAL_CAN_ERROR_EPV)) {
    return CAN_HW_BUS_STATUS_ERROR;
  }

  return CAN_HW_BUS_STATUS_OK;
}

StatusCode can_hw_transmit(uint32_t id, bool extended, const uint8_t *data, uint8_t len) {
  if (data == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (len > 8U) {
    return STATUS_CODE_INVALID_ARGS;
  }

  CAN_TxHeaderTypeDef tx_header = {
    .StdId = id,
    .ExtId = id,
    .IDE = extended ? CAN_ID_EXT : CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = len,
    .TransmitGlobalTime = DISABLE
  };

  uint32_t tx_mailbox;

  HAL_StatusTypeDef status;
  TickType_t timeout = pdMS_TO_TICKS(MAX_TX_MS_TIMEOUT);

  for (size_t i = 0; i < MAX_TX_RETRIES; ++i) {
    status = HAL_CAN_AddTxMessage(&s_can_handle, &tx_header, data, &tx_mailbox);
    
    if (status == HAL_OK) {
      return STATUS_CODE_OK;
    }
    else if (status == HAL_BUSY) {
      // Wait for mailbox to free up
      if (xSemaphoreTake(s_can_tx_ready_sem_handle, timeout) != pdTRUE) {
        LOG_WARN("CAN TX timeout");
        continue;
      }
    }
    else {
      LOG_CRITICAL("CAN TX error: %d", status);
      return STATUS_CODE_INTERNAL_ERROR;
    }
  }
  
  return STATUS_CODE_RESOURCE_EXHAUSTED;
}

bool can_hw_receive(uint32_t *id, bool *extended, uint64_t *data, uint8_t *len) {
  if (id == NULL || extended == NULL || data == NULL || len == NULL) {
    return false;
  }

  CAN_RxHeaderTypeDef rx_header;
  uint8_t rx_data[8];

  /* Check FIFO 0 then FIFO 1 for data */
  if (HAL_CAN_GetRxFifoFillLevel(&s_can_handle, CAN_RX_FIFO0) > 0) {
    if (HAL_CAN_GetRxMessage(&s_can_handle, CAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK) {
      *extended = (rx_header.IDE == CAN_ID_EXT);
      *id = *extended ? rx_header.ExtId : rx_header.StdId;
      *len = rx_header.DLC;
      memcpy(data, rx_data, rx_header.DLC);
      return true;
    }
  } else if (HAL_CAN_GetRxFifoFillLevel(&s_can_handle, CAN_RX_FIFO1) > 0) {
    if (HAL_CAN_GetRxMessage(&s_can_handle, CAN_RX_FIFO1, &rx_header, rx_data) == HAL_OK) {
      *extended = (rx_header.IDE == CAN_ID_EXT);
      *id = *extended ? rx_header.ExtId : rx_header.StdId;
      *len = rx_header.DLC;
      memcpy(data, rx_data, rx_header.DLC);
      return true;
    }
  }

  return false;
}

void CAN1_TX_IRQHandler(void) {
  HAL_CAN_IRQHandler(&s_can_handle);
}

void CAN1_RX0_IRQHandler(void) {
  HAL_CAN_IRQHandler(&s_can_handle);
}

void CAN1_RX1_IRQHandler(void) {
  HAL_CAN_IRQHandler(&s_can_handle);
}

void CAN1_SCE_IRQHandler(void) {
  HAL_CAN_IRQHandler(&s_can_handle);
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan) {
  if (s_tx_full) {
    /* TX mailbox is no longer full once data is transmitted */
    xSemaphoreGiveFromISR(s_can_tx_ready_sem_handle, NULL);
    s_tx_full = false;
  }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  BaseType_t higher_woken = pdFALSE;
  CanMessage rx_msg = { 0 };
  
  if (can_hw_receive(&rx_msg.id.raw, &rx_msg.extended, &rx_msg.data, &rx_msg.dlc)) {
    bool s_filter_id_match = false;
    for (uint32_t i = 0; i < CAN_HW_NUM_FILTER_BANKS; i++) {
      /* Check if the ID is in the filter */
      if (can_filters[i] == rx_msg.id.raw) {
        s_filter_id_match = true;
        break;
      }
    }

    /* If the ID is not in the filters, push to RX queue */
    if (!s_filter_id_match) {
      can_queue_push_from_isr(s_g_rx_queue, &rx_msg, &higher_woken);
    }
  }
  
  portYIELD_FROM_ISR(higher_woken);
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  BaseType_t higher_woken = pdFALSE;
  CanMessage rx_msg = { 0 };
  
  if (can_hw_receive(&rx_msg.id.raw, &rx_msg.extended, &rx_msg.data, &rx_msg.dlc)) {
    bool s_filter_id_match = false;
    for (uint32_t i = 0; i < CAN_HW_NUM_FILTER_BANKS; i++) {
      /* Check if the ID is in the filter */
      if (can_filters[i] == rx_msg.id.raw) {
        s_filter_id_match = true;
        break;
      }
    }

    /* If the ID is not in the filters, push to RX queue */
    if (!s_filter_id_match) {
      can_queue_push_from_isr(s_g_rx_queue, &rx_msg, &higher_woken);
    }
  }
  
  portYIELD_FROM_ISR(higher_woken);
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan) {
  /* TODO: Add error notifications/handling */
  /* Aryan - Maybe reinitialize bus? */
  uint32_t error = HAL_CAN_GetError(hcan);
  
  if (error & HAL_CAN_ERROR_BOF) {
    HAL_CAN_ResetError(hcan);
    HAL_CAN_Start(hcan);
  }
}
