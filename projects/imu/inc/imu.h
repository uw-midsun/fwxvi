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
#include "can.h"
#include "can_board_ids.h"
#include "spi.h"

/* Intra-component Headers */

/**
 * @defgroup imu
 * @brief    imu Firmware
 * @{
 */

/** @} */

typedef enum { REG_BANK_SEL = 0x7F, GYRO_REG_ADDR = 0x06, ACCEL_REG_ADDR = 0x03 } bmi323_registers;

typedef enum { ACC_DP_OFF_X = 0x60, ACC_DP_DGAIN_X = 0x61, ACC_DP_OFF_Y = 0x62, ACC_DP_DGAIN_Y = 0x63, ACC_DP_OFF_Z = 0x64, ACC_DP_DGAIN_Z = 0x65 } accel_go_registers;

typedef enum { GYR_DP_OFF_X = 0x66, GYR_DP_DGAIN_X = 0x67, GYR_DP_OFF_Y = 0x68, GYR_DP_DGAIN_Y = 0x69, GYR_DP_OFF_Z = 0x6A, GYR_DP_DGAIN_Z = 0x6B } gyro_go_registers;

typedef enum { IMU_ACCEL_RANGE_2G, IMU_ACCEL_RANGE_4G, IMU_ACCEL_RANGE_8G, IMU_ACCEL_RANGE_16G, NUM_IMU_ACCEL_RANGE } IMUAccelRange;

typedef enum { IMU_GYRO_RANGE_125_DEG, IMU_GYRO_RANGE_250_DEG, IMU_GYRO_RANGE_500_DEG, IMU_GYRO_RANGE_1000_DEG, IMU_GYRO_RANGE_2000_DEG, NUM_IMU_GYRO_RANGES } IMUGyroRange;

typedef struct {
  SpiPort spi_port;
  SpiBaudrate spi_baudrate;
  GpioAddress mosi;
  GpioAddress miso;
  GpioAddress sclk;
  GpioAddress cs;
  GpioAddress int1;
  GpioAddress int2;
  IMUAccelRange accel_range;
  IMUGyroRange gyro_range;
} IMUSettings;

typedef struct {
  int16_t x;
  int16_t y;
  int16_t z;
} axes;

typedef struct {
  uint16_t accel_offset_x;
  uint16_t accel_offset_y;
  uint16_t accel_offset_z;
  uint8_t accel_gain_x;
  uint8_t accel_gain_y;
  uint8_t accel_gain_z;
} accel_gain_offset_values;

typedef struct {
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

StatusCode imu_init(IMUSettings *settings);
static StatusCode set_register(uint16_t reg_addr, uint16_t value);
static StatusCode set_multi_register(uint8_t reg_addr, uint8_t *value, uint16_t len);
static StatusCode get_register(bmi323_registers reg, uint8_t *value);
static StatusCode get_multi_register(bmi323_registers reg, uint8_t *reg_val, uint8_t len);
static StatusCode get_gyroscope_data(axes *gyro);
static StatusCode get_accel_data(axes *accel);
static StatusCode enable_feature_engine();
