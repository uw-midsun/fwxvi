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
#include "imu.h"
#include "sd_card_spi.h"
#include "telemetry.h"
#include "telemetry_getters.h"
#include "telemetry_hw_defs.h"
#include "xb_transmit.h"

TelemetryStorage telemetry_storage;

TelemetryConfig telemetry_config = {
  .message_transmit_frequency_hz = 1000U,
  .uart_port = TELEMETRY_XBEE_UART_PORT,
  .uart_settings = { .tx = GPIO_TELEMETRY_UART_TX, .rx = GPIO_TELEMETRY_UART_RX, .baudrate = TELEMETRY_XBEE_UART_BAUDRATE, .flow_control = TELEMETRY_XBEE_UART_FLOW_CONTROL },
  .sd_spi_port = SPI_PORT_2,
  .sd_spi_settings = { .baudrate = SD_SPI_BAUDRATE_2_5MHZ, // all correct from darren, also not necessary to check
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
                    .sdo = GPIO_TELEMETRY_SPI_MOSI,
                    .sdi = GPIO_TELEMETRY_SPI_MISO,
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

float roll = 0;
float pitch = 0;
float yaw = 0;
void pre_loop_init() {
  imu_init(&bmi323_storage, &bmi323_settings);
  // for (float i = 0; i < 1000; i++) {
  //   imu_filter(0.05, 0.05, 0.9, 0, 0, 0);
  //   eulerAngles(q_est, &roll, &pitch, &yaw);
  // }
}

void run_1000hz_cycle() {}

void run_10hz_cycle() {
  //run_can_tx_medium();
  imu_run();
  LOG_DEBUG("IMU: x: %u | y: %u | z: %u\r\n", (uint16_t)telemetry_storage.bmi323_storage->gyro.x * 100, (uint16_t)telemetry_storage.bmi323_storage->gyro.y * 100, (uint16_t)telemetry_storage.bmi323_storage->gyro.z * 100);
  
}

void run_1hz_cycle() {
  //run_can_tx_slow();
}

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
int main(int argc, char *argv[]) {
  mpxe_init(argc, argv);
#else
int main() {
#endif
  mcu_init(); // works
  tasks_init(); // works
  log_init(); // works
  // works

  telemetry_init(&telemetry_storage, &telemetry_config, &bmi323_storage, &can_storage);
  init_master_tasks();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
