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
#include "tasks.h"
#include "uart.h"

/* Intra-component Headers */
#include "telemetry_getters.h"
#include "telemetry.h"

TelemetryStorage telemetry_storage;

TelemetryConfig telemetry_config = { .message_transmit_frequency_hz = 1000U,
                                     .uart_port = UART_PORT_2,
                                     .uart_settings = {
                                         .tx = { .port = GPIO_PORT_A, .pin = 2 }, .rx = { .port = GPIO_PORT_A, .pin = 3 }, .baudrate = 115200, .flow_control = UART_FLOW_CONTROL_NONE } };

Bmi323Storage bmi323_storage = { 0 };

void pre_loop_init() {}

void run_1000hz_cycle() {
  run_can_rx_all();
}

void run_10hz_cycle() {
  run_can_tx_medium();
  printf("SL: %d, DSS: %d, DSF: %d, PP: %ld\r\n", get_steering_buttons_lights(), get_steering_buttons_drive_state(), get_pedal_data_drive_state(), get_pedal_percentage());
}

void run_1hz_cycle() {
  run_can_tx_slow();
}

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
int main(int argc, char *argv[]) {
  mpxe_init(argc, argv);
#else
int main() {
#endif
  mcu_init();
  tasks_init();
  log_init();

  telemetry_init(&telemetry_storage, &telemetry_config, &bmi323_storage);

  init_master_tasks();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
