/************************************************************************************************
 * @file    imu.c
 *
 * @brief   Imu
 *
 * @date    2026-01-04
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>  // For memcpy

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "bmi323.h"
#include "datagram.h"
#include "imu.h"
#include "log.h"  // For LOG_ERROR
#include "tasks.h"

/* Intra-component Headers */

TASK(imu_task, TASK_STACK_1024) {
  Bmi323Storage *storage = (Bmi323Storage *)context;

  const TickType_t xPeriod = pdMS_TO_TICKS(100);

  while (1) {
    vTaskDelay(xPeriod);

    StatusCode status = bmi323_update(storage);
    if (status != STATUS_CODE_OK) {
      LOG_CRITICAL("status error");
      continue;
    }

    // Create buffer and log IMU values
    uint8_t buffer[26];
    buffer[0] = DATAGRAM_START_FRAME;
    memcpy(&buffer[1], &storage->gyro.x, sizeof(float));
    memcpy(&buffer[5], &storage->gyro.y, sizeof(float));
    memcpy(&buffer[9], &storage->gyro.z, sizeof(float));
    memcpy(&buffer[13], &storage->accel.x, sizeof(float));
    memcpy(&buffer[17], &storage->accel.y, sizeof(float));
    memcpy(&buffer[21], &storage->accel.z, sizeof(float));
    buffer[25] = DATAGRAM_END_FRAME;

    LOG_DEBUG("IMU gx=%.3f gy=%.3f gz=%.3f ax=%.3f ay=%.3f az=%.3f\n", (double)storage->gyro.x, (double)storage->gyro.y, (double)storage->gyro.z, (double)storage->accel.x, (double)storage->accel.y,
              (double)storage->accel.z);
  }
}

StatusCode imu_init(Bmi323Storage *storage, Bmi323Settings *settings) {
  storage->settings = settings;

  StatusCode status = bmi323_init(storage);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  tasks_init_task(imu_task, TASK_PRIORITY(1), storage);

  return STATUS_CODE_OK;
}
