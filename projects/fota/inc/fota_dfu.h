#pragma once

/************************************************************************************************
 * @file  fota_dfu.h
 *
 * @brief  FOTA Device Firmware Update
 *
 * @date   2025-07-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/


/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_datagram.h"
#include "fota_error.h"
#include "packet_manager.h"

/**
 * @brief   DFU internal states
 */
typedef enum {
  FOTA_DFU_UNINITIALIZED,   /**< DFU Uninitialized */
  FOTA_DFU_IDLE,            /**< DFU Idle */
  FOTA_DFU_RECEIVING,       /**< DFU Recieving data */
  FOTA_DFU_COMPLETE,        /**< DFU Complete */
  FOTA_DFU_JUMP             /**< DFU Jump memory */
} FotaDFUState;

/**
 * @brief   DFU Context for writing firmware to flash
 */
typedef struct {
  bool is_initialized;            /**< Initialization flag */
  uintptr_t flash_base_addr;      /**< Flash bank where new firmware is written */
  uintptr_t app_start_addr;       /**< Main application entry address (for jump) */
  uintptr_t current_write_addr;   /**< Current write address */
  uint32_t  bytes_written;        /**< Total bytes written so far */
  uint32_t  binary_size;          /**< Expected size of firmware binary */
  uint16_t  expected_datagram_id; /**< Monotonic ID for validating sequencing */
  FotaDFUState  state;            /**< Current DFU FSM state */
  PacketManager *packet_manager;  /**< Pointer to packet manager */
} FotaDFUContext;

/**
 * @brief   Initialize DFU context
 * @param   packet_manager Pointer to packet manager instance
 * @param   staging_base Flash base address where firmware will be staged
 * @param   app_start_addr Address to jump to after flashing
 * @return  FOTA_ERROR_SUCCESS or FOTA_ERROR_INVALID_ARGS
 */
FotaError fota_dfu_init(PacketManager *packet_manager, uintptr_t staging_base, uintptr_t app_start_addr);

/**
 * @brief   Process an incoming datagram and write to flash
 * @param   datagram Pointer to fully received, verified datagram
 * @return  FOTA_ERROR_SUCCESS or flash/write errors
 */
FotaError fota_dfu_process(FotaDatagram *datagram);

/**
 * @brief   Finalizes the DFU and copies binary from staging area to application region
 * @return  FOTA_ERROR_SUCCESS or copy error
 */
FotaError fota_dfu_finalize();

