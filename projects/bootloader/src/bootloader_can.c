/************************************************************************************************
 * @file   bootloader_can.c
 *
 * @brief  Source file for CAN communication in the bootloader
 *
 * @date   2025-01-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "bootloader_can.h"

/* CAN has 3 transmit mailboxes and 2 receive FIFOs */
#define CAN_HW_BASE CAN1
#define MAX_TX_RETRIES 3
#define MAX_TX_MS_TIMEOUT 20

static CAN_HandleTypeDef s_can_handle;

static BootCanTiming s_timing[NUM_BOOT_CAN_BITRATES] = {
  [BOOT_CAN_BITRATE_125KBPS] = { .prescaler = 40, .bs1 = CAN_BS1_12TQ, .bs2 = CAN_BS2_1TQ },
  [BOOT_CAN_BITRATE_250KBPS] = { .prescaler = 20, .bs1 = CAN_BS1_12TQ, .bs2 = CAN_BS2_1TQ },
  [BOOT_CAN_BITRATE_500KBPS] = { .prescaler = 10, .bs1 = CAN_BS1_12TQ, .bs2 = CAN_BS2_1TQ },
  [BOOT_CAN_BITRATE_1000KBPS] = { .prescaler = 5, .bs1 = CAN_BS1_12TQ, .bs2 = CAN_BS2_1TQ }
};

BootloaderError boot_can_init(const Boot_CanSettings *settings) {

  if (settings == NULL) return BOOTLOADER_INVALID_ARGS;

  __HAL_RCC_CAN1_CLK_ENABLE();


  uint32_t can_mode = CAN_MODE_NORMAL;
  can_mode |= (settings->loopback)? CAN_MODE_LOOPBACK: 0x0;
  can_mode |= (settings->silent)? CAN_MODE_SILENT: 0x0;

  s_can_handle.Instance = (CAN_HandleTypeDef *) settings->device_id;
  s_can_handle.State = HAL_CAN_STATE_READY;
  s_can_handle.Init.Prescaler = s_timing[settings->bitrate].prescaler;
  s_can_handle.Init.Mode = can_mode;
  s_can_handle.Init.SyncJumpWidth = CAN_SJW_1TQ;
  s_can_handle.Init.TimeSeg1 = s_timing[settings->bitrate].bs1;
  s_can_handle.Init.TimeSeg2 = s_timing[settings->bitrate].bs2;
  s_can_handle.Init.AutoBusOff = ENABLE;                          /* Auto error handling. Turns bus off when many errors detected */
  s_can_handle.Init.AutoWakeUp = DISABLE;                         /* Node stays in sleep until explicitly woken  */
  s_can_handle.Init.AutoRetransmission = DISABLE;                 /* We use one-shot transmission since auto retransmit can cause timing issues */  
  s_can_handle.Init.ReceiveFifoLocked = DISABLE;                  /* Ensures latest data is always available */
  s_can_handle.Init.TransmitFifoPriority = DISABLE;
  s_can_handle.ErrorCode = HAL_CAN_ERROR_NONE;

  if (HAL_CAN_Init(&s_can_handle) != HAL_OK) {
    return BOOTLOADER_INTERNAL_ERR;
  }

  if (HAL_CAN_Start(&s_can_handle) != HAL_OK) {
    return BOOTLOADER_INTERNAL_ERR;
  }

  return BOOTLOADER_ERROR_NONE;
}

BootloaderError boot_can_transmit(uint32_t id, bool extended, const uint8_t *data, size_t len) {

  if (s_can_handle.State != HAL_CAN_STATE_READY) return BOOTLOADER_ERROR_UNINITIALIZED;

  if (data == NULL) {
    return BOOTLOADER_INVALID_ARGS;
  }

  if (len > 8U) {
    return BOOTLOADER_INVALID_ARGS;
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
  for (size_t i = 0; i < MAX_TX_RETRIES; ++i) {
    if (HAL_CAN_AddTxMessage(&s_can_handle, &tx_header, (uint8_t*)data, &tx_mailbox) != HAL_OK) {
      return BOOLOADER_CAN_TRANSMISSION_ERROR;
    }
  }
  
  return BOOTLOADER_ERROR_NONE;
}

BootloaderError boot_can_receive(Boot_CanMessage *msg) {
  
  if (s_can_handle.State != HAL_CAN_STATE_READY) return BOOTLOADER_ERROR_UNINITIALIZED;

  if (msg->id == NULL || msg->extended == NULL || msg->dlc == NULL || msg->data == NULL){
    return BOOTLOADER_CAN_RECEIVE_ERROR;
  }

  CAN_RxHeaderTypeDef rx_header;
  uint8_t rx_data[8];

  if (HAL_CAN_GetRxFifoFillLevel(&s_can_handle, CAN_RX_FIFO0) > 0) {
    if (HAL_CAN_GetRxMessage(&s_can_handle, CAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK) {
      msg->extended = (rx_header.IDE == CAN_ID_EXT);
      msg->id = msg->extended ? rx_header.ExtId : rx_header.StdId;
      msg->dlc = rx_header.DLC;

      // how to know with data to use? there are 4 diff types in union 
      memcpy(msg->data, rx_data, rx_header.DLC);
      return BOOTLOADER_ERROR_NONE;
    }
  } else if (HAL_CAN_GetRxFifoFillLevel(&s_can_handle, CAN_RX_FIFO1) > 0) {
    if (HAL_CAN_GetRxMessage(&s_can_handle, CAN_RX_FIFO1, &rx_header, rx_data) == HAL_OK) {
      msg->extended = (rx_header.IDE == CAN_ID_EXT);
      msg->id = msg->extended  ? rx_header.ExtId : rx_header.StdId;
      msg->dlc = rx_header.DLC;
      memcpy(msg->data, rx_data, rx_header.DLC);
      return BOOTLOADER_ERROR_NONE;
    }
  }

  return BOOTLOADER_FLASH_READ_FAILED;
}
