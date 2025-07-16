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

#define FOTA_CHUNK_SIZE 2048

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

typedef struct {
  uint16_t prescaler;
  uint32_t bs1;
  uint32_t bs2;
} BootCanTiming;

typedef enum {
  BOOT_CAN_BITRATE_125KBPS,  /**< 125 KBits per second */
  BOOT_CAN_BITRATE_250KBPS,  /**< 250 KBits per second */
  BOOT_CAN_BITRATE_500KBPS,  /**< 500 KBits per second */
  BOOT_CAN_BITRATE_1000KBPS, /**< 1000 KBits per second */
  NUM_BOOT_CAN_BITRATES      /**< Number of supported bit rates */
} Boot_CanBitrate;

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
typedef struct CanSettings {
  uint16_t device_id;      /**< Device CAN ID */
  Boot_CanBitrate bitrate; /**< Bits per second */
  bool loopback;           /**< Enables self-listening for message debugging */
  bool silent;             /**< Device can listen but not transmit messages */
} Boot_CanSettings;

/**
 * @brief   Bootloader CAN State Machine
 */
typedef enum {
  CAN_STATE_READY,
  CAN_STATE_WAIT_FOR_START,
  CAN_STATE_RECIEVE_CHUNK,
  CAN_STATE_VALIDATE_CHUNK,
  CAN_STATE_WRITE_CHUNK,
  CAN_STATE_UPDATE,
  CAN_STATE_JUMP,
  CAN_STATE_APP,
  CAN_STATE_ACK,
  CAN_STATE_ERROR
} Can_StateMachine;

/**
 * @brief   FOTA CAN Command Identifiers
 */
typedef enum { FOTA_CMD_START = 0x01, FOTA_CMD_CHUNK = 0x02, FOTA_CMD_END = 0x03, FOTA_CMD_ACK = 0x04, FOTA_CMD_ERROR = 0x06 } FotaCommand;

FotaError can_bootloader_init(void);

FotaError can_bootloader_transmit(uint32_t id, bool extended, uint8_t *data, uint8_t len);

FotaError can_bootloader_receive(uint32_t id, uint8_t *data, uint8_t *len);

FotaError can_bootloader_poll(void);

FotaError can_bootloader_chunkify(void);

/**
 * @brief   Initialize the CAN interface for the bootloader
 * @return  FOTA_ERROR_SUCCESS if the interface is initialized succesfully
 *          FOTA_ERROR_CAN_INIT if initialization fails
 */
FotaError boot_can_init(const Boot_CanSettings *settings);

/**
 * @brief   Transmit a CAN message in the bootloader
 * @param   id CAN Message arbitration Id
 * @param   extended Boolean flag to select 11-bit vs 29-bit CAN transmission
 * @param   data Pointer to the data buffer to transmit
 * @param   len Length of the data buffer
 * @return  BOOTLOADER_ERROR_NONE if the message is transmitted succesfully
 *          BOOLOADER_CAN_TRANSMISSION_ERROR if transmitting the message fails
 */
FotaError boot_can_transmit(uint32_t id, bool extended, const uint8_t *data, size_t len);

/**
 * @brief   Receive a CAN message in the bootloader
 * @param   msg Pointer to the message that will be updated with incoming data
 * @return  BOOTLOADER_ERROR_NONE if the message is received succesfully
 *          BOOTLOADER_CAN_RECEIVE_ERROR if receiving the message fails
 */
FotaError boot_can_receive(Fota_CanMessage *const msg);

/** @} */
