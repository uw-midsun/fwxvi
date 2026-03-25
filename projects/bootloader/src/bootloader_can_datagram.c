/************************************************************************************************
 * @file   bootloader_can_datagram.c
 *
 * @brief  Source file for the CAN datagram in the bootloader
 *
 * @date   2025-01-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "bootloader_can_datagram.h"

/**
 * @brief Buffer to store error messages over CAN
 */
uint8_t error_buffer[DGRAM_MAX_MSG_SIZE]; 

BootloaderDatagram deserialize_datagram(Boot_CanMessage *msg, uint16_t *target_nodes) {
  BootloaderDatagram ret_datagram;
  do {
    switch (msg->id) {
      case BOOTLOADER_CAN_START_ID:
        ret_datagram.datagram_type_id = msg->id;
        ret_datagram.payload.start.node_ids = msg->data_u16[0];
        *target_nodes = msg->data_u16[0];
        ret_datagram.payload.start.data_len =
            ((uint32_t)msg->data_u16[2] << 16) | (uint32_t)msg->data_u16[1];
        break;
      case BOOTLOADER_CAN_SEQUENCING_ID:
        ret_datagram.datagram_type_id = msg->id;
        ret_datagram.payload.sequencing.sequence_num = msg->data_u16[0];
        ret_datagram.payload.sequencing.crc32 =
            ((uint32_t)msg->data_u16[2] << 16) | (uint32_t)msg->data_u16[1];
        break;
      case  BOOTLOADER_CAN_FLASH_ID:
        ret_datagram.datagram_type_id = msg->id;
        ret_datagram.payload.data.data = msg->data_u8;
        break;
      case BOOTLOADER_CAN_JUMP_APPLICATION_ID:
        ret_datagram.payload.jump_app.node_ids = msg->data_u16[0];
        *target_nodes = msg->data_u16[0];
        break;
      case BOOTLOADER_CAN_PING_DATA_ID:
        ret_datagram.payload.ping.node_ids = msg->data_u16[0];
        *target_nodes = msg->data_u16[0];
        ret_datagram.payload.ping.req = (msg->data_u16[1] >> 12) & 0xF;
        ret_datagram.payload.ping.data_len = msg->data_u16[1] & 0xFFF;
        break;
      default:
        break;
    }
  } while (false);
  return ret_datagram;
}

void send_ack_datagram(bool ack, BootloaderError error) {
  error_buffer[0] = ack;
  uint16_t status_code = error;
  error_buffer[1] = (uint8_t)(error << 8);
  error_buffer[2] = (uint8_t)(error);
  boot_can_transmit(BOOTLOADER_CAN_ACK_ID, false, error_buffer, sizeof(error_buffer));
}
