/************************************************************************************************
 * @file   fota.c
 *
 * @brief  Source file for fota
 *
 * @date   2025-03-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_dfu.h"
#include "fota_encryption.h"
#include "fota_error.h"
#include "fota_startup.h"
#include "network.h"
#include "network_buffer.h"
#include "packet_manager.h"

#define FOTA_RF_BOARD_NODE_ID 0U

static PacketManager packet_manager;
static UartSettings network_uart_settings = { .tx = { .port = GPIO_PORT_A, .pin = 2U }, .rx = { .port = GPIO_PORT_A, .pin = 3U }, .baudrate = 115200U, .flow_control = UART_FLOW_CONTROL_NONE };

static void fota_datagram_complete_cb(FotaDatagram *datagram) {
  switch (datagram->header.target_node_id) {
    case FOTA_RF_BOARD_NODE_ID: {
      /* Trigger DFU */
      if (fota_dfu_process(datagram) != FOTA_ERROR_SUCCESS) {
        /* Generate NACK response */
      }
    } break;

    default: {
      /* Trigger CAN Bootloader */
    }
  }
}

FotaError fota_init() {
  fota_startup();

  fota_encryption_init();

  packet_manager_init(&packet_manager, UART_PORT_2, &network_uart_settings, fota_datagram_complete_cb);

  return FOTA_ERROR_SUCCESS;
}

void fota_process(void) {
  packet_manager_process(&packet_manager);
}
