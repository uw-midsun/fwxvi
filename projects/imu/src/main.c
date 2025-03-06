/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for imu
 *
 * @date   2025-02-01
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */
#include "imu.h"
#include "imu_hw_defs.h"
#include "bmi323.h"

Bmi323Storage bmi323_storage;

Bmi323Settings bmi323_settings = {
  .spi_port = SPI_PORT_2,
  .spi_settings = {
    .baudrate = SPI_BAUDRATE_1_25MHZ,
    .mode = SPI_MODE_3,
    .sdo = IMU_SPI_SDO,
    .sdi = IMU_SPI_SDI,
    .sclk = IMU_SPI_SCLK,
    .cs = IMU_SPI_NSS,
  },
  .accel_range = IMU_ACCEL_RANGE_2G,
  .gyro_range = IMU_GYRO_RANGE_500_DEG,
};

void pre_loop_init() {}

void run_1000hz_cycle() {}

void run_10hz_cycle() {

}

void run_1hz_cycle() {}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  imu_init(&bmi323_storage, &bmi323_settings);

  init_master_tasks();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
