#pragma once

/************************************************************************************************
 * @file   bootloader.h
 *
 * @brief  Header file for bootloader
 *
 * @date   2025-01-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "bootloader_can.h"
#include "bootloader_error.h"

/**
 * @defgroup bootloader
 * @brief    bootloader Firmware
 * @{
 */
extern uint32_t _flash_start;
extern uint32_t _flash_size;
extern uint32_t _flash_page_size;
extern uint32_t _application_start;
extern uint32_t _application_size;
extern uint32_t _bootloader_start;
extern uint32_t _bootloader_size;
extern uint32_t _sram_start;
extern uint32_t _sram_size;

/** @brief  Flash start address as defined in the linkerscripts */
#define FLASH_START_ADDRESS ((uint32_t)&_flash_start)

/** @brief  Flash size as defined in the linkerscripts */
#define FLASH_SIZE ((size_t)&_flash_size)

/** @brief  Flash page size as defined in the linkerscripts */
#define FLASH_PAGE_SIZE ((size_t)&_flash_page_size)

/** @brief  Application start address as defined in the linkerscripts */
#define APPLICATION_START_ADDRESS ((uint32_t)&_application_start)

/** @brief  Application size as defined in the linkerscripts */
#define APPLICATION_SIZE ((size_t)&_application_size)

/** @brief  Bootloader start address as defined in the linkerscripts */
#define BOOTLOADER_START_ADDRESS ((uint32_t)&_bootloader_start)

/** @brief  Bootloader size as defined in the linkerscripts */
#define BOOTLOADER_SIZE ((size_t)&_bootloader_size)

/** @brief  SRAM start address as defined in the linkerscripts */
#define SRAM_START_ADDRESS ((uint32_t)&_sram_start)

/** @brief  SRAM size as defined in the linkerscripts */
#define SRAM_SIZE ((size_t)&_sram_size)

/**
 * @brief   Bootloader State Machine
 */
typedef enum {
  BOOTLOADER_UNINITIALIZED = 0, /**< Bootloader starts as unitialized */
  BOOTLOADER_IDLE,              /**< Bootloader is initialized and waiting for messages */
  BOOTLOADER_START,             /**< Bootloader is processing a start message */
  BOOTLOADER_WAIT_SEQUENCING,   /**< Bootloader is waiting for the data sequencing packet */
  BOOTLOADER_DATA_RECEIVE,      /**< Bootloader is receiving streamed data and flashing it in 2048 byte chunks */
  BOOTLOADER_JUMP_APP,          /**< Bootloader is prompted to jump to application defined by APPLICATION_START_ADDRESS */
  BOOTLOADER_FAULT              /**< Bootloader is in fault state */
} BootloaderStates;

/**
 * @brief   Private Bootloader State Storage
 */
typedef struct {
  uintptr_t application_start;       /**< Application start address */
  uintptr_t current_write_address;   /**< Current write address */
  uint32_t bytes_written;            /**< Number of bytes written to flash */
  uint32_t data_size;                /**< Data size (ie: Binary application size) */
  uint32_t packet_crc32;             /**< Packet CRC32 if available */
  uint16_t expected_sequence_number; /**< Next expected sequence number for validation */
  uint16_t buffer_index;             /**< Data buffer index for correct reading/writing */

  BootloaderStates state;   /**< Internal state tracker */
  BootloaderError error;    /**< Bootloader error tracker */
  uint16_t target_nodes;    /**< Target MCU Ids */
  bool first_byte_received; /**< Boolean flag to track if the first byte was received */
} BootloaderStateData;

/**
 * @brief   Initialize the bootloader
 * @return  BOOTLOADER_ERROR_NONE if the bootloader is initialized succesfully
 */
BootloaderError bootloader_init(void);

/**
 * @brief   Run the bootloader
 * @param   msg Pointer to incoming CAN data to feed the bootloader
 * @return  BootloaderError collected through the state machine
 */
BootloaderError bootloader_run(Boot_CanMessage *msg);

/**
 * @brief   Jump application
 * @details Exits bootloader and jumps to main application
 * @return  BOOTLOADER_INTERNAL_ERR. This function should never return
 */
BootloaderError bootloader_jump_app(void);

/** @} */
