/************************************************************************************************
 * can_hw.c
 *
 * Source file for CAN HW Interface
 *
 * Created: 2024-11-03
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
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
#define MAX_TX_RETRIES 3
#define MAX_TX_MS_TIMEOUT 20

/* STM32L4 has 14 filter banks */
#define CAN_HW_NUM_FILTER_BANKS 14

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
/* For 80 MHz APB1 clock */
static CanHwTiming s_timing[NUM_CAN_HW_BITRATES] = {
  [CAN_HW_BITRATE_125KBPS] = { .prescaler = 40, .bs1 = CAN_BS1_12TQ, .bs2 = CAN_BS2_1TQ },
  [CAN_HW_BITRATE_250KBPS] = { .prescaler = 20, .bs1 = CAN_BS1_12TQ, .bs2 = CAN_BS2_1TQ },
  [CAN_HW_BITRATE_500KBPS] = { .prescaler = 10, .bs1 = CAN_BS1_12TQ, .bs2 = CAN_BS2_1TQ },
  [CAN_HW_BITRATE_1000KBPS] = { .prescaler = 5, .bs1 = CAN_BS1_12TQ, .bs2 = CAN_BS2_1TQ }
};

static uint8_t s_num_filters;
static CanQueue *s_g_rx_queue;
static CAN_HandleTypeDef s_can_handle;

static SemaphoreHandle_t s_can_tx_ready_sem_handle;
static StaticSemaphore_t s_can_tx_ready_sem;
static bool s_tx_full = false;

// takes 1 for filter_in, 2 for filter_out and default is 0
static int s_can_filter_en = 0;
static uint32_t can_filters[CAN_HW_NUM_FILTER_BANKS];

static void prv_add_filter_in(uint8_t filter_num, uint32_t mask, uint32_t filter) {
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
  gpio_init_pin_af(&settings->tx, GPIO_ALTFN_PUSH_PULL, GPIO_ALT9_CAN1);
  gpio_init_pin_af(&settings->rx, GPIO_ALTFN_PUSH_PULL, GPIO_ALT9_CAN1);

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
  s_can_handle.Init.SyncJumpWidth = CAN_SJW_1TQ;
  s_can_handle.Init.TimeSeg1 = s_timing[settings->bitrate].bs1;
  s_can_handle.Init.TimeSeg2 = s_timing[settings->bitrate].bs2;
  s_can_handle.Init.TimeTriggeredMode = DISABLE;
  s_can_handle.Init.AutoBusOff = ENABLE;
  s_can_handle.Init.AutoWakeUp = DISABLE;
  s_can_handle.Init.AutoRetransmission = ENABLE;
  s_can_handle.Init.ReceiveFifoLocked = DISABLE;
  s_can_handle.Init.TransmitFifoPriority = DISABLE;

  if (HAL_CAN_Init(&s_can_handle) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  
  /* Enable all interrupts */
  HAL_CAN_ActivateNotification(&s_can_handle, CAN_IT_TX_MAILBOX_EMPTY |
                                          CAN_IT_RX_FIFO0_MSG_PENDING |
                                          CAN_IT_RX_FIFO1_MSG_PENDING |
                                          CAN_IT_ERROR);
  
  interrupt_nvic_enable(CAN1_TX_IRQn, INTERRUPT_PRIORITY_HIGH);
  interrupt_nvic_enable(CAN1_RX0_IRQn, INTERRUPT_PRIORITY_HIGH);
  interrupt_nvic_enable(CAN1_RX1_IRQn, INTERRUPT_PRIORITY_HIGH);
  interrupt_nvic_enable(CAN1_SCE_IRQn, INTERRUPT_PRIORITY_HIGH);

  /* Allow all messages by default, but reset the filter count so it's overwritten on the first filter */
  prv_add_filter_in(0, 0, 0);
  s_num_filters = 0;

  /* Initialize CAN queue */
  s_g_rx_queue = rx_queue;

  /* Create available mailbox semaphore */
  s_can_tx_ready_sem_handle = xSemaphoreCreateBinaryStatic(&s_can_tx_ready_sem);
  configASSERT(s_can_tx_ready_sem_handle);
  s_tx_full = false;

  if (HAL_CAN_Start(&s_can_handle) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  LOG_DEBUG("CAN HW initialized on %s\n", CAN_HW_DEV_INTERFACE);

  return STATUS_CODE_OK;
}
