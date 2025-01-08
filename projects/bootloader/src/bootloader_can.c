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
  return BOOTLOADER_ERROR_NONE;
}

BootloaderError boot_can_transmit(uint32_t id, bool extended, const uint8_t *data, size_t len) {
  return BOOTLOADER_ERROR_NONE;
}

BootloaderError boot_can_receive(Boot_CanMessage *msg) {
  return BOOTLOADER_ERROR_NONE;
}
