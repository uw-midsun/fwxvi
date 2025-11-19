#pragma once

/************************************************************************************************
 * @file    bootloader_dfu.h
 *
 * @brief   Bootloader Dfu
 *
 * @date    2025-10-05
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "bootloader_error.h"
#include "bootloader.h"
#include "bootloader_can_datagram.h"
#include "bootloader_packet_handler.h"

/**
 * @defgroup bootloader_dfu
 * @brief    bootloader_dfu Firmware
 * @{
 */

/** @brief Stores info needed for the DFU process */
typedef struct {
  uintptr_t application_start;       /**< Application start address */
  uintptr_t current_write_address;   /**< Current write address */
} BootloaderDFUInfo;

/**
 * @brief   Initializes the `BootloaderDFUInfo` struct
 * @param   current_write_address The current write address
 * @return  `BOOTLOADER_ERROR_NONE` on success
 */
BootloaderError bootloader_dfu_init(uintptr_t current_write_address);

/**
 * @brief   Flash page onto flash memory
 * @details Called by `bootloader_receive_data` function in bootloader_packet_handler.c
 * @param   pm Pointer to the packet manager
 * @param   datagram CAN datagram
 * @param   flash_buffer Flash buffer which should have 2KB of data
 * @return  `BOOTLOADER_ERROR_NONE` on success
 */
BootloaderError bootloader_dfu_flash(PacketManager *pm, uint8_t *flash_buffer);

/**
 * @brief   Jumps to application vector table
 * @return  Should never return
 */
BootloaderError bootloader_jump_app(); 

/**
 * @brief   If any errors/faults arise, reset the board
 * @return  BootloaderError 
 */
BootloaderError bootloader_fault();
/** @} */
