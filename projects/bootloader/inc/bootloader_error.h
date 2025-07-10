#pragma once

/************************************************************************************************
 * @file   bootloader_error.h
 *
 * @brief  Header file for the Bootloader errors
 *
 * @date   2025-01-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup bootloader
 * @brief    bootloader Firmware
 * @{
 */

/**
 * @brief   Bootloader error definitions
 */
typedef enum {
  BOOTLOADER_ERROR_NONE = 0,            /**< No error is detected */
  BOOTLOADER_OVERSIZE,                  /**< More data is received than expected */
  BOOTLOADER_DATA_NOT_ALIGNED,          /**< Data is not 4-byte aligned as per ARM32 spec */
  BOOTLOADER_INVALID_ID,                /**< Invalid authenication ID */
  BOOTLOADER_INTERNAL_ERR,              /**< Unidentified internal controller error*/
  BOOTLOADER_INVALID_ARGS,              /**< Invalid arguments are passed into the bootloader */
  BOOTLOADER_ERROR_UNINITIALIZED,       /**< Bootloader is uninitialized */
  BOOTLOADER_FLASH_ERR,                 /**< Flash write or erase failure */
  BOOTLOADER_FLASH_WRITE_NOT_ALIGNED,   /**< Bootloader flash write is not aligned */
  BOOTLOADER_FLASH_WRITE_OUT_OF_BOUNDS, /**< Bootloader flash write adddress is out of bounds */
  BOOTLOADER_FLASH_READ_FAILED,         /**< Bootloader flash read failed*/
  BOOTLOADER_BUFFER_OVERFLOW,           /**< Bootloader flash buffer has overflowed (More than 2048 bytes) */
  BOOTLOADER_CAN_INIT_ERR,              /**< Bootloader CAN initialization failed */
  BOOLOADER_CAN_TRANSMISSION_ERROR,     /**< CAN Bus transmission failed */
  BOOTLOADER_CAN_RECEIVE_ERROR,         /**< CAN Bus receive failed */
  BOOTLOADER_CAN_TRANSMIT_ERROR,        /**< CAN Bus receive failed */
  BOOTLOADER_INVALID_ADDRESS,           /**< Attempted to write/read to an incorrect address */
  BOOTLOADER_CRC_MISMATCH_BEFORE_WRITE, /**< Bootloader has detected a CRC32 mismatch before writing to flash */
  BOOTLOADER_CRC_MISMATCH_AFTER_WRITE,  /**< Bootloader has detected a CRC32 mismatch after writing to flash */
  BOOTLOADER_SEQUENCE_ERROR,            /**< Bootloader has received an incorrect sequence number */
  BOOTLOADER_TIMEOUT,                   /**< Bootloader has timed out */
  BOOTLOADER_FLASH_MEMORY_VERIFY_FAILED,/**< Bootloader flash memory verification failed. Everything is still erased */
  BOOTLOADER_PING_TIMEOUT               /**< Bootloader ping timed out */
} BootloaderError;

// test again 
/** @} */
