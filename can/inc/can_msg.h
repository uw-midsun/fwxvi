#pragma once

/************************************************************************************************
 * @file   can_msg.h
 *
 * @brief  CAN message definitions
 *
 * @date   2024-11-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup CAN
 * @brief    CAN library
 * @{
 */

/**
 * @brief   CAN message ID
 * @details 11 Bits in standard mode
 *          29 Bits in extended mode
*/
typedef uint32_t CanMessageId;

/**
 * @brief   CAN message ID
 */
typedef union CanId {
  CanMessageId raw;
  struct {
    uint16_t source_id : 4;
    uint16_t type : 1;
    uint32_t msg_id : 24;
  };
} CanId;

/**
 * @brief   CAN message structure
 * @details CAN ID is subjected to arbitration where lower ID's are given higher priority
 *          Extended ID flag indicates a 29-bit ID or an 11-bit ID
 *          Data length code ias 4 bit value indicating the number of bytes in the payload
 *          CAN Frames hold 8 bytes of data at most
 */
typedef struct CanMessage {
  CanId id;                 /**< CAN ID */
  bool extended;            /**< Extended ID Flag */
  size_t dlc;               /**< Data length Code */
  union {
    uint64_t data;          /**< Raw data in the form of 8 bytes */
    uint32_t data_u32[2];   /**< Data split in 4 bytes partitions */
    uint16_t data_u16[4];   /**< Data split in 2 bytes partitions */
    uint8_t data_u8[8];     /**< Data split in 1 byte partitions */
  };
} CanMessage;

/** @} */
