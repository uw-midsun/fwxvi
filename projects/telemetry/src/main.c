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
#include "sd_card_spi.h"
#include "telemetry.h"
#include "telemetry_getters.h"
#include "telemetry_hw_defs.h"
#include "xb_transmit.h"

TelemetryStorage telemetry_storage;

TelemetryConfig telemetry_config = {
  .message_transmit_frequency_hz = 1000U,
  .uart_port = UART_PORT_2,
  .uart_settings = { .tx = GPIO_TELEMETRY_UART_TX, .rx = GPIO_TELEMETRY_UART_RX, .baudrate = 115200, .flow_control = UART_FLOW_CONTROL_NONE },
  .sd_spi_port = SPI_PORT_2,
  .sd_spi_settings = { .baudrate = SD_SPI_BAUDRATE_2_5MHZ,
                       .mode = SD_SPI_MODE_1,
                       .mosi = GPIO_TELEMETRY_SPI_MOSI,
                       .miso = GPIO_TELEMETRY_SPI_MISO,
                       .sclk = GPIO_TELEMETRY_SPI_SCK,
                       .cs = GPIO_TELEMETRY_SPI_NSS,
                      },
};

Bmi323Settings bmi323_settings = {
  .spi_port = SPI_PORT_2,
  .spi_settings = { .baudrate = SPI_BAUDRATE_5MHZ,
                    .mode = SPI_MODE_3,
                    .sdo = GPIO_TELEMETRY_SPI_MISO,
                    .sdi = GPIO_TELEMETRY_SPI_MOSI,
                    .sclk = GPIO_TELEMETRY_SPI_SCK,
                    .cs = GPIO_TELEMETRY_SPI_NSS,
                  },
  .accel_range = IMU_ACCEL_RANGE_2G,
  .gyro_range = IMU_GYRO_RANGE_500_DEG,
};

Bmi323Storage bmi323_storage = {
  .settings = &bmi323_settings,
};

CanStorage can_storage = { 0 };

uint8_t datagram_data[] = { 0, 1, 2, 3, 4 };

static Datagram tx_datagram = {
  .start_frame = 0xAA,
  .id = 0x01,
  .dlc = 5,
};

static size_t datagram_length = 0;

static StatusCode status = STATUS_CODE_OK;

void pre_loop_init() {}

void run_1000hz_cycle() {
  run_can_rx_all();
  // xb_transmit_run();
}

void run_10hz_cycle() {
  run_can_tx_medium();
  // imu_run();

  // datagram_length = tx_datagram.dlc + DATAGRAM_METADATA_SIZE;
  // for (int i = 0; i < 5; i++) {
  //   tx_datagram.data[i] = i;
  // }

  // status = uart_tx(UART_PORT_2, (uint8_t *)&tx_datagram, datagram_length);

  // printf("uart_tx with return code %d\r\n", status);
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

  telemetry_init(&telemetry_storage, &telemetry_config, &bmi323_storage, &can_storage);

  init_master_tasks();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
