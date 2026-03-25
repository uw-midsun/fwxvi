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
#include "bootloader_flash.h"
#include "bootloader_memory_map.h"

/**
 * @defgroup bootloader
 * @brief    bootloader Firmware
 * @{
 */

/**
 * @brief   Bootloader State Machine
 */
typedef enum {
  BOOTLOADER_UNINITIALIZED = 0, /**< Bootloader starts as unitialized */
  BOOTLOADER_IDLE,              /**< Bootloader is initialized and waiting for messages */
  BOOTLOADER_START,             /**< Bootloader is processing a start message */
  BOOTLOADER_WAIT_SEQUENCING,   /**< Bootloader is waiting for the data sequencing packet */
  BOOTLOADER_DATA_RECEIVE,      /**< Bootloader is receiving streamed data and flashing it in 2048 byte chunks */
  BOOTLOADER_JUMP_APP,          /**< Bootloader is prompted to jump to application defined by APP_ACTIVE_START_ADDRESS */
  BOOTLOADER_FAULT,             /**< Bootloader is in fault state */
  BOOTLOADER_PING               /**< Bootloader is ready to start receiving data */
} BootloaderStates;

/**
 * @brief Bootloader Ping States
 */
typedef enum {
  /// @brief Bootloader should start pinging MCU's
  BOOTLOADER_PING_NODES = 0,
  /// @brief Bootloader ping should do branch stuff
  BOOTLOADER_PING_BRANCH,
  /// @brief Bootloader ping should do group stuff
  BOOTLOADER_PING_PROJECT,
} BootloaderPingStates;

/**
 * @brief   Initialize the bootloader
 * @return  BOOTLOADER_ERROR_NONE if the bootloader is initialized successfully
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
