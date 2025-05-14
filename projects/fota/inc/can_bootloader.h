#pragma once

/************************************************************************************************
 * @file    can_bootloader.h
 *
 * @brief   CAN interface for the FOTA bootloader
 *
 * @date    2025-05-14
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_error.h"

/**
 * @defgroup FOTA
 * @brief    Firmware Over the Air (FOTA) Update system
 * @{
 */

 
 /**
 * @brief   CAN message structure
 * @details CAN ID is subjected to arbitration where lower ID's are given higher priority
 *          Extended ID flag indicates a 29-bit ID or an 11-bit ID
 *          Data length code ias 4 bit value indicating the number of bytes in the payload
 *          CAN Frames hold 8 bytes of data at most
 */
typedef struct {
  uint32_t id;      /**< CAN ID */
  uint8_t extended; /**< Extended ID Flag */
  size_t dlc;       /**< Data length Code */

  union {
    uint64_t data;        /**< Raw data in the form of 8 bytes */
    uint32_t data_u32[2]; /**< Data split in 4 bytes partitions */
    uint16_t data_u16[4]; /**< Data split in 2 bytes partitions */
    uint8_t data_u8[8];   /**< Data split in 1 byte partitions */
  };

} Fota_CanMessage;


FotaError can_bootloader_init(void);

FotaError can_bootloader_transmit(uint32_t id, bool extended, uint8_t *data, uint8_t len);

FotaError can_bootloader_receive(uint32_t id, uint8_t *data, uint8_t *len);

FotaError can_bootloader_poll(void);

FotaError can_bootloader_chunkify(void);

// state machine (switch) function + struct
