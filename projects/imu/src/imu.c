/************************************************************************************************
 * @file   imu.c
 *
 * @brief  driver file for imu
 *
 * @date   2025-02-01
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdio.h>

/* Inter-component Headers */
#include "status.h"
#include "system_can.h"
#include "can.h"

/* Intra-component Headers */
#include "imu.h"
#include "imu_hw_defs.h"

static CanStorage s_can_storage = { 0U };
const CanSettings s_can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_IMU,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .tx = IMU_CAN_TX,
    .rx = IMU_CAN_RX,
    .loopback = 0,
};

StatusCode imu_init(Bmi323Storage *storage, Bmi323Settings *settings) {
  if (storage == NULL || settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  storage->settings = settings;

  status_ok_or_return(spi_init(settings->spi_port, &settings->spi_settings));

//   status_ok_or_return(can_init(&s_can_storage, &s_can_settings));

  status_ok_or_return(bmi323_init(storage));

  return STATUS_CODE_OK;
}
