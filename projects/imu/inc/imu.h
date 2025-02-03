#pragma once

/************************************************************************************************
 * @file   imu.h
 *
 * @brief  Header file for imu
 *
 * @date   2025-02-01
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include <spi.h>

/* Intra-component Headers */

/**
 * @defgroup imu
 * @brief    imu Firmware
 * @{
 */

/** @} */

typedef struct {
  SpiPort spi_port;
  uint32_t spi_baudrate;
  GpioAddress mosi;
  GpioAddress miso;
  GpioAddress sclk;
  GpioAddress cs;
  GpioAddress int1;
  GpioAddress int2;
} IMUSettings;

typedef struct {
  int16_t x;
  int16_t y;
  int16_t z;
} axes;


typedef struct{
  uint16_t accel_offset_x;
  uint16_t accel_offset_y;
  uint16_t accel_offset_z;
  uint8_t accel_gain_x;
  uint8_t accel_gain_y;
  uint8_t accel_gain_z;
} accel_gain_offset_values;

typedef struct{
  uint16_t gyro_offset_x;
  uint16_t gyro_offset_y;
  uint16_t gyro_offset_z;
  uint8_t gyro_gain_x;
  uint8_t gyro_gain_y;
  uint8_t gyro_gain_z;
} gyro_gain_offset_values;


typedef struct {
  IMUSettings *settings;
  axes accel;
  axes gyro;
  accel_gain_offset_values accel_go_values;
  gyro_gain_offset_values gyro_go_values;
} IMUStorage;