#pragma once

/************************************************************************************************
 * @file    bootloader_packet_handler.h
 *
 * @brief   Bootloader Packet Handler
 *
 * @date    2025-10-05
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "bootloader.h"
#include "bootloader_can_datagram.h"

/**
 * @defgroup bootloader_packet_handler
 * @brief    bootloader_packet_handler Firmware
 * @{
 */

/**
 * @brief   Stores info related to receiving data
 */
typedef struct {
  uintptr_t current_write_address;   /**< Current write address */                   
  uint32_t bytes_written;            /**< Number of bytes written to flash */               
  uint32_t data_size;                /**< Data size (ie: Binary application size) */        
  uint32_t packet_crc32;             /**< Packet CRC32 if available */                      
  uint16_t expected_sequence_number; /**< Next expected sequence number for validation */   
  uint16_t buffer_index;             /**< Data buffer index for correct reading/writing */  
  BootloaderPingStates ping_type;    /**< Ping state of bootloader */                       
  uint8_t ping_data_len;             /**< Length of ping data */                            
  uint16_t target_nodes;             /**< Target MCU Ids */                                          
  bool first_byte_received;          /**< Boolean flag to track if the first byte was received */   
  bool first_ping_received;          /**< Boolean flag to track if the first ping was received */   
} PacketManager; 

/**
 * @brief   Initialize the `PacketManager` struct
 * @param   pm A pointer to the `PacketManager` struct
 * @return  BOOTLOADER_ERROR_NONE on success
 */
BootloaderError packet_handler_init(PacketManager *pm);

/**
 * @brief   Function call for the `BOOTLOADER_START` state
 * @details Primes everything to start receiving data
 * @param   pm Pointer to the `PacketManager` struct
 * @param   datagram A pointer to the incoming datagram
 * @return  BOOTLOADER_DATA_NOT_ALIGNED, BOOTLOADER_ERROR_NONE
 */
BootloaderError bootloader_start(PacketManager *pm, BootloaderDatagram *datagram);

/**
 * @brief   Function call for the `BOOTLOADER_DATA_RECEIVE` state
 * @details Receives incoming chunks, if buffer is the size of one page, flash onto board
 * @param   pm Pointer to the `PacketManager` struct
 * @param   datagram A pointer to the incoming datagram
 * @param   flash_buffer The current buffer to be flashed
 * @return  BOOTLOADER_ERROR_NONE if successful 
 */
BootloaderError bootloader_receive_data(PacketManager *pm, BootloaderDatagram *datagram, uint8_t *flash_buffer);

/**
 * @brief   Function call for `BOOTLOADER_WAIT_SEQUENCING` state
 * @details Validates datagram packets, in terms of sequencing, to ensure packets were not missed
 * @param   pm Pointer to the `PacketManager` struct
 * @param   datagram Pointer to incoming datagram
 * @return  BOOTLOADER_ERROR_NONE if successful else BOOTLOADER_SEQUENCE_ERROR
 */
BootloaderError bootloader_wait_sequencing(PacketManager *pm, BootloaderDatagram *datagram);

/**
 * @brief   Function call for the `BOOTLOADER_PING` state
 * @details Sends over misc data (Not yet completed)
 * @param   pm Believe it or not, it is a pointer to the `PacketManager` struct
 * @param   datagram Pointer to the incoming datagram
 * @param   flash_buffer The current buffer to be flashed
 * @return  BOOTLOADER_ERROR_NONE if successful
 */
BootloaderError bootloader_ping(PacketManager *pm, BootloaderDatagram *datagram, uint8_t *flash_buffer);

/** @} */
