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

BootloaderDatagram deserialize_datagram(Boot_CanMessage *msg, uint16_t *target_nodes) {
  BootloaderDatagram ret_datagram = { 0 };

  return ret_datagram;
}

void send_ack_datagram(bool ack, BootloaderError error) {}
