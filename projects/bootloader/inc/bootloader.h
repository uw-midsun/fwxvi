#pragma once

#include "boot_can.h"
#include "boot_flash.h"
#include "bootloader_mcu.h"
#include "can_datagram.h"

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
#define FLASH_START_ADDRESS_LINKERSCRIPT ((uint32_t)&_flash_start)

/** @brief  Flash size as defined in the linkerscripts */
#define FLASH_SIZE_LINKERSCRIPT ((size_t)&_flash_size)

/** @brief  Flash page size as defined in the linkerscripts */
#define FLASH_PAGE_SIZE_LINKERSCRIPT ((size_t)&_flash_page_size)

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
  /// @brief Bootloader starts unitialized
  BOOTLOADER_UNINITIALIZED = 0,
  /// @brief Bootloader is initialized and waiting for messages
  BOOTLOADER_IDLE,
  /// @brief Bootloader is processing a start message
  BOOTLOADER_START,
  /// @brief Bootloader is waiting for the first byte to be sent (CAN_ARBITRATION_SEQUENCING_ID)
  BOOTLOADER_DATA_READY,
  /// @brief Bootloader is receiving streamed data and flashing it immediately
  /// (CAN_ARBITRATION_FLASH_ID)
  BOOTLOADER_DATA_RECEIVE,
  /// @brief Bootloader is prompted to jump to application defined by APP_START_ADDRESS
  BOOTLOADER_JUMP_APP,
  /// @brief Bootloader is in fault state
  BOOTLOADER_FAULT
} BootloaderStates;

typedef struct {
  uintptr_t application_start;
  uintptr_t current_address;
  uint32_t bytes_written;
  uint32_t binary_size;
  uint32_t packet_crc32;
  uint16_t expected_sequence_number;
  uint16_t buffer_index;

  BootloaderStates state;
  BootloaderError error;
  uint16_t target_nodes;
  bool first_byte_received;

} BootloaderStateData;

BootloaderError bootloader_init(void);
BootloaderError bootloader_run(Boot_CanMessage *msg);
BootloaderError bootloader_jump_app(void);
