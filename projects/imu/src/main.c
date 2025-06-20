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
#include "can.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */
#include "bmi323.h"
#include "imu.h"
#include "imu_hw_defs.h"

Bmi323Storage bmi323_storage;

Bmi323Settings bmi323_settings = {
  .spi_port = IMU_SPI_PORT,
  .spi_settings = {
    .baudrate = IMU_SPI_BAUDRATE,
    .mode = IMU_SPI_MODE,
    .sdo = IMU_SPI_SDO,
    .sdi = IMU_SPI_SDI,
    .sclk = IMU_SPI_SCLK,
    .cs = IMU_SPI_NSS,
  },
  .accel_range = IMU_ACCEL_RANGE_2G,
  .gyro_range = IMU_GYRO_RANGE_500_DEG,
};

void pre_loop_init() {
  imu_init(&bmi323_storage, &bmi323_settings);
}

void run_1000hz_cycle() {}

void run_10hz_cycle() {
  bmi323_update(&bmi323_storage);
  LOG_DEBUG("BMI323 ACCEL - x: %d%%, y: %d%%, z: %d%%\r\n", (int)(bmi323_storage.accel.x * 100), (int)(bmi323_storage.accel.y * 100), (int)(bmi323_storage.accel.z * 100));
}

void run_1hz_cycle() {}

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

  init_master_tasks();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
