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

BootloaderError boot_can_init(const Boot_CanSettings *settings) {
  if (settings == NULL) return BOOTLOADER_INVALID_ARGS;

  __HAL_RCC_CAN1_CLK_ENABLE();

  CAN_HandleTypeDef hcan;

  uint32_t can_mode = CAN_MODE_NORMAL;
  can_mode |= (settings->loopback)? CAN_MODE_LOOPBACK: 0x0;
  can_mode |= (settings->silent)? CAN_MODE_SILENT: 0x0;

  hcan.Instance = (CAN_HandleTypeDef *) settings->device_id;
  hcan.State = HAL_CAN_STATE_READY;
  hcan.Init.Prescaler = s_timing[settings->bitrate].prescaler;
  hcan.Init.Mode = can_mode;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = s_timing[settings->bitrate].bs1;
  hcan.Init.TimeSeg1 = s_timing[settings->bitrate].bs2;
  hcan.Init.AutoBusOff = ENABLE;                          /* Auto error handling. Turns bus off when many errors detected */
  hcan.Init.AutoWakeUp = DISABLE;                         /* Node stays in sleep until explicitly woken  */
  hcan.Init.AutoRetransmission = DISABLE;                 /* We use one-shot transmission since auto retransmit can cause timing issues */  
  hcan.Init.ReceiveFifoLocked = DISABLE;                  /* Ensures latest data is always available */
  hcan.Init.TransmitFifoPriority = DISABLE;
  hcan.ErrorCode = HAL_CAN_ERROR_NONE;

  if (HAL_CAN_Init(&hcan) != HAL_OK) {
    return BOOTLOADER_INTERNAL_ERR;
  }
  

  return BOOTLOADER_ERROR_NONE;
}

BootloaderError boot_can_transmit(uint32_t id, bool extended, const uint8_t *data, size_t len) {
  return BOOTLOADER_ERROR_NONE;
}

BootloaderError boot_can_receive(Boot_CanMessage *msg) {
  return BOOTLOADER_ERROR_NONE;
}
