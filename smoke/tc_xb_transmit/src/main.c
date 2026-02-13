/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for Telemetry XB Transmit
 *
 * @date   2026-02-11
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "ff.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */
#include "datagram.h"
#include "global_enums.h"
#include "telemetry.h"
#include "telemetry_hw_defs.h"

static TelemetryStorage *telemetry_storage;

static TelemetryConfig s_telemetry_config = {
  .uart_port = UART_PORT_2,
  .uart_settings = {
    .baudrate = 115200,
  },
  .message_transmit_frequency_hz = 10U,
};

uint8_t datagram_data[] = { 0, 1, 2, 3, 4 };

static Datagram tx_datagram = {
  .start_frame = 0xAA,
  .id = 0x01,
  .dlc = 5,
};

static size_t datagram_length = 0;

TASK(xb_transmit, TASK_STACK_1024) {
  Datagram datagram = { 0U };
  CanMessage message = { 0U };
  uint16_t msg_count = 0;
  StatusCode status = STATUS_CODE_OK;

  LOG_DEBUG("Telemetry smoke test");
  delay_ms(1000U);

  while (true) {
    datagram_length = tx_datagram.dlc + DATAGRAM_METADATA_SIZE;
    for (int i = 0; i < 5; i++) {
      tx_datagram.data[i] = i;
    }

    status = uart_tx(UART_PORT_2, (uint8_t *)&tx_datagram, datagram_length);

    printf("uart_tx with return code %d\r\n", status);
    delay_ms(10U);
  }
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

  tasks_init_task(xb_transmit, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
