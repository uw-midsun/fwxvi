/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for IMU run
 *
 * @date   2026-03-18
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "ff.h"
#include "gpio.h"
#include "imu.h"
#include "log.h"
#include "mcu.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */
#include "datagram.h"
#include "global_enums.h"
#include "telemetry.h"
#include "telemetry_hw_defs.h"

TelemetryStorage telemetry_storage;

TelemetryConfig telemetry_config = {
  .message_transmit_frequency_hz = 1000U,
  .uart_port = TELEMETRY_XBEE_UART_PORT,
  .uart_settings = { .tx = GPIO_TELEMETRY_UART_TX, .rx = GPIO_TELEMETRY_UART_RX, .baudrate = TELEMETRY_XBEE_UART_BAUDRATE, .flow_control = TELEMETRY_XBEE_UART_FLOW_CONTROL },
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

TASK(IMU_RUN, 1024) {
  imu_init(&bmi323_storage, &bmi323_settings);

  telemetry_storage.bmi323_storage = &bmi323_storage;
  while (true) {
    LOG_DEBUG("Starting IMU run\n");
    StatusCode status = imu_run();

    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Error occurred");
      delay_ms(1000U);
    } else if (status == STATUS_CODE_OK) {
      LOG_DEBUG("IMU gx=%.3f gy=%.3f gz=%.3f ax=%.3f ay=%.3f az=%.3f\n", (double)bmi323_storage.gyro.x, (double)bmi323_storage.gyro.y, (double)bmi323_storage.gyro.z, (double)bmi323_storage.accel.x,
                (double)bmi323_storage.accel.y, (double)bmi323_storage.accel.z);
      delay_ms(1000U);
    }
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

  tasks_init_task(IMU_RUN, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
