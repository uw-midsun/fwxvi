#pragma once

/************************************************************************************************
 * @file   fota_can.h
 *
 * @brief  Header file for CAN communication in FOTA
 *
 * @date   2025-01-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>
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
 * @brief   Selection for the supported CAN Bitrates
 * @details CAN bit timing is composed of multiple time segments:
 *          - Synchronization Segment (Sync_Seg): Allows resynchronization
 *          - Propagation Segment (Prop_Seg): Compensates for physical delays
 *          - Phase Buffer Segments (Phase_Seg1 and Phase_Seg2):
 *            * Absorb time quanta variations
 *            * Provide sampling point adjustment
 * @note    Typical sampling occurs between 75-80% of bit time
 *          Higher bitrates require shorter time quantas
 *          Bitrate selection impacts signal reliability and bus length
 */
typedef struct {
  uint16_t prescaler;
  uint32_t bs1;
  uint32_t bs2;
} FotaCanTiming;

typedef enum {
  FOTA_CAN_BITRATE_125KBPS,  /**< 125 KBits per second */
  FOTA_CAN_BITRATE_250KBPS,  /**< 250 KBits per second */
  FOTA_CAN_BITRATE_500KBPS,  /**< 500 KBits per second */
  FOTA_CAN_BITRATE_1000KBPS, /**< 1000 KBits per second */
  NUM_FOTA_CAN_BITRATES      /**< Number of supported bit rates */
} FotaCanBitrate;

typedef enum { CAN_CONTINUOUS = 0, CAN_ONE_SHOT_MODE, NUM_CAN_MODES } Boot_CanMode;

#define CAN_HW_BASE CAN1

/**
 * @brief   CAN Settings
 * @details TX Pin will transmit data. RX pin will receive data.
 *          The bitrate is the number of bits sent per second.
 *          The device ID is the CAN ID of the STM32 node.
 *          Loopback will internally connect the transmit and receive CAN lines for testing.
 *          Silent mode is used for only listening to the bus.
 */
typedef struct FotaCanSettings {
  uint16_t device_id;     /**< Device CAN ID */
  FotaCanBitrate bitrate; /**< Bits per second */
  bool loopback;          /**< Enables self-listening for message debugging */
  bool silent;            /**< Device can listen but not transmit messages */
} FotaCanSettings;

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
} FotaCanMessage;

/**
 * @brief   Initialize the CAN interface for FOTA
 * @return  FOTA_ERROR_SUCCESS if the interface is initialized successfully
 *          FOTA_CAN_INIT_ERR if initialization fails
 */
FotaError fota_can_init(const FotaCanSettings *settings);

/**
 * @brief   Transmit a CAN message in FOTA
 * @param   id CAN Message arbitration Id
 * @param   extended Boolean flag to select 11-bit vs 29-bit CAN transmission
 * @param   data Pointer to the data buffer to transmit
 * @param   len Length of the data buffer
 * @return  FOTA_ERROR_SUCCESS if the message is transmitted successfully
 *          FOTA_CAN_TRANSMISSION_ERROR if transmitting the message fails
 */
FotaError fota_can_transmit(uint32_t id, bool extended, const uint8_t *data, size_t len);

/**
 * @brief   Receive a CAN message in FOTA
 * @param   msg Pointer to the message that will be updated with incoming data
 * @return  FOTA_ERROR_SUCCESS if the message is received successfully
 *          FOTA_CAN_RECEIVE_ERROR if receiving the message fails
 */
FotaError fota_can_receive(FotaCanMessage *const msg);

/** @} */
