/************************************************************************************************
 * @file    bootloader_dfu.c
 *
 * @brief   Bootloader Dfu
 *
 * @date    2025-10-05
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "stm32l4xx.h"

/* Intra-component Headers */
#include "bootloader_dfu.h"
#include "bootloader_flash.h"
#include "bootloader_crc32.h"

BootloaderDFUInfo dfu_info; 

BootloaderError bootloader_dfu_init(uintptr_t current_write_address) {
  dfu_info.application_start = APP_ACTIVE_START_ADDRESS; 
  dfu_info.current_write_address = current_write_address; 
  return BOOTLOADER_ERROR_NONE; 
}

BootloaderError bootloader_dfu_flash(PacketManager *pm, uint8_t *flash_buffer) {
  BootloaderError error = BOOTLOADER_ERROR_NONE; 

  uint32_t calculated_crc32 = boot_crc32_calculate((const uint32_t *)flash_buffer, BYTES_TO_WORD(pm->buffer_index));
  if (calculated_crc32 != pm->packet_crc32) {
    send_ack_datagram(NACK, BOOTLOADER_CRC_MISMATCH_BEFORE_WRITE);
    return BOOTLOADER_CRC_MISMATCH_BEFORE_WRITE;
  }
  
  error |= boot_flash_erase(BOOTLOADER_ADDR_TO_PAGE(dfu_info.current_write_address), 1U);
  error |= boot_flash_write(dfu_info.current_write_address, flash_buffer, BOOTLOADER_PAGE_BYTES);
  error |= boot_flash_read(dfu_info.current_write_address, flash_buffer, BOOTLOADER_PAGE_BYTES);
  
  calculated_crc32 = boot_crc32_calculate((uint32_t *)flash_buffer, BYTES_TO_WORD(pm->buffer_index));
  if (calculated_crc32 != pm->packet_crc32) {
    send_ack_datagram(NACK, BOOTLOADER_CRC_MISMATCH_AFTER_WRITE);
    return BOOTLOADER_CRC_MISMATCH_AFTER_WRITE;
  }
  if (error != BOOTLOADER_ERROR_NONE) {
    send_ack_datagram(NACK, error);
    return error;
  }

  return error;
}

BootloaderError bootloader_jump_app() {
  send_ack_datagram(ACK, BOOTLOADER_ERROR_NONE);
  __asm volatile(
      "LDR     R0, =dfu_info  \n"
      "LDR     R1, [R0]             \n"
      "LDR     R2, [R1, #4]         \n"
      "BX      R2                   \n");

  return BOOTLOADER_INTERNAL_ERR;
}

BootloaderError bootloader_fault() {
  send_ack_datagram(NACK, BOOTLOADER_FAULT); 
  NVIC_SystemReset(); 
  /* Just a reset, could pivot to something else if needed */ 
  return BOOTLOADER_INTERNAL_ERR;
}
