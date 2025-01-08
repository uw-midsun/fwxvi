#pragma once

/************************************************************************************************
 * @file   bootloader_can_datagram.h
 *
 * @brief  Header file for the CAN datagram in the bootloader
 *
 * @date   2025-01-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
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

/** @brief  ACK defined as 0 */
#define ACK 0U

/** @brief  NACK defined as 1 */
#define NACK 1U

/**
 * @brief   Bootloader reserved CAN arbitration IDs
 */
typedef enum {
  BOOTLOADER_CAN_SEQUENCING_ID = 30,  /**< Data sequencing command (Highest priority) */
  BOOTLOADER_CAN_FLASH_ID,            /**< Incoming flash data command */
  BOOTLOADER_CAN_JUMP_APPLICATION_ID, /**< Jump to application command */
  BOOTLOADER_CAN_ACK_ID,              /**< Bootloader ACK/NACK message */
  BOOTLOADER_CAN_START_ID,            /**< Start DFU command */
  BOOTLOADER_CAN_JUMP_BOOTLOADER      /**< Jump to bootloader command */
} BootloaderCanID;

/**
 * @brief   Datagram message definition
 */
typedef struct {
  uint8_t datagram_type_id; /**< Message Id (CAN arbitration Id) */

  /**
   * @brief     CAN Payload union
   */
  union {
    /**
     * @brief   Start message definition
     */
    struct {
      uint16_t node_ids; /**< Target MCU Id */
      uint32_t data_len; /**< Incoming data length (ie: Binary size) */
    } start;

    /**
     * @brief   Sequencing message definition
     */
    struct {
      uint16_t sequence_num; /**< Sequence number */
      uint32_t crc32;        /**< Flash chunk CRC32 checksum */
    } sequencing;

    /**
     * @brief   Data message definition
     */
    struct {
      uint8_t *data; /**< Pointer to buffer containing message data */
    } data;

    /**
     * @brief   Jump application message definition
     */
    struct {
      uint16_t node_ids; /**< Target MCU Id */
    } jump_app;

    /**
     * @brief   ACK/NACK message definition
     */
    struct {
      uint8_t ack_status;        /**< 0: ACK 1: NACK */
      uint16_t bootloader_error; /**< Bootloader error as per BootloaderError definition */
    } ack;
  } payload;
} BootloaderDatagram;

/**
 * @brief   Deserialize an incoming CAN message
 * @details This will deserialize a CAN message and produce a datagram
 * @param   msg Pointer to the incoming CAN message
 * @param   target_nodes Pointer that will be updated with the target MCU Id
 * @return  Updated datagram with new data
 */
BootloaderDatagram deserialize_datagram(Boot_CanMessage *msg, uint16_t *target_nodes);

/**
 * @brief   Send Bootloader ACK/NACK message over CAN
 * @param   ack Boolean ACK status. 0: ACK, 1: NACK
 * @param   error Bootloader error status for further information
 */
void send_ack_datagram(bool ack, BootloaderError error);

/** @} */
