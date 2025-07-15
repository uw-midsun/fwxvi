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
#include "fota_encryption.h"
#include "fota_error.h"
#include "network.h"
#include "network_buffer.h"
#include "packet_manger.h"

static PacketManager packet_manager;

static void datagram_complete_callback(FotaDatagram *datagram) {
  // TODO handle callback
}

FotaError fota_init() {
  static UartSettings uart2_settings = {
    .tx = { .port = GPIO_PORT_A, .pin = GPIO_PIN_2 }, .rx = { .port = GPIO_PORT_A, .pin = GPIO_PIN_3 }, .baudrate = 115200, .flow_control = UART_FLOW_CONTROL_NONE
  };

  fota_encryption_init();

  packet_manager_init(&packet_manager, &uart2_settings, datagram_complete_callback);

  return FOTA_ERROR_SUCCESS;
}

void fota_process(void) {
  packet_manager_process(&packet_manager);
}
