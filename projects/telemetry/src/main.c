/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for telemetry
 *
 * @date   2025-01-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "can.h"
#include "datagram.h"
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "system_can.h"
#include "tasks.h"
#include "uart.h"

/* Intra-component Headers */
#include "telemetry.h"

TelemetryStorage telemetry_storage;

TelemetryConfig telemetry_config = { .message_transmit_frequency_hz = 1000U,
                                     .uart_port = UART_PORT_2,
                                     .uart_settings = {
                                         .tx = { .port = GPIO_PORT_A, .pin = 2 }, .rx = { .port = GPIO_PORT_A, .pin = 3 }, .baudrate = 115200, .flow_control = UART_FLOW_CONTROL_NONE } };

int main() {
  mcu_init();
  tasks_init();

  telemetry_init(&telemetry_storage, &telemetry_config);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
