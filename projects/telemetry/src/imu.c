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
#include "log.h"
#include "tasks.h"

/* Intra-component Headers */

#define IMU_DEBUG 0U

static Bmi323Storage *s_storage = NULL;

StatusCode imu_run() {
  StatusCode status = bmi323_update(s_storage);
  if (status != STATUS_CODE_OK) {
    LOG_CRITICAL("status error");
    return status;
  }
#if (IMU_DEBUG == 1)
  // Create buffer and log IMU values
  uint8_t buffer[26];
  buffer[0] = DATAGRAM_START_FRAME;
  memcpy(&buffer[1], &s_storage->gyro.x, sizeof(float));
  memcpy(&buffer[5], &s_storage->gyro.y, sizeof(float));
  memcpy(&buffer[9], &s_storage->gyro.z, sizeof(float));
  memcpy(&buffer[13], &s_storage->accel.x, sizeof(float));
  memcpy(&buffer[17], &s_storage->accel.y, sizeof(float));
  memcpy(&buffer[21], &s_storage->accel.z, sizeof(float));
  buffer[25] = DATAGRAM_END_FRAME;

  LOG_DEBUG("IMU gx=%.3f gy=%.3f gz=%.3f ax=%.3f ay=%.3f az=%.3f\n", (double)s_storage->gyro.x, (double)s_storage->gyro.y, (double)s_storage->gyro.z, (double)s_storage->accel.x,
            (double)s_storage->accel.y, (double)s_storage->accel.z);
#endif
  return STATUS_CODE_OK;
}

StatusCode imu_init(Bmi323Storage *storage, Bmi323Settings *settings) {
  if (storage == NULL || settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_storage = storage;
  s_storage->settings = settings;

  StatusCode status = bmi323_init(s_storage);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  return STATUS_CODE_OK;
}
