#pragma once

/************************************************************************************************
 * @file   bmi323.h
 *
 * @brief  Header file for BMI323 Driver
 *
 * @date   2025-03-06
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "can.h"
#include "spi.h"

/* Intra-component Headers */

/**
 * @defgroup imu
 * @brief    imu Firmware
 * @{
 */

#define FEATURE_IO0 0x10
#define FEATURE_IO1 0x11
#define FEATURE_IO2 0x12
#define FEATURE_IO3 0x13
#define FEATURE_IO_STATUS 0x14
#define FEATURE_CTRL 0x40
#define INT_STATUS_INT1 0x0D
#define INT_STATUS_INT2 0x0E

#define BMI_SET_LOW_BYTE 0x00FF
#define BMI_SET_HIGH_BYTE 0xFF00
#define BMI3_E_NULL_PTR INT8_C(-1)

#define BMI323_CHIP_ID          0x43U
#define BMI323_MAX_NUM_DATA     16U

/*
this might be 13 or 14 idk
#define BMI3_ACC_DP_OFF_XYZ_13_BIT_MASK              UINT16_C(0x1FFF)
#define BMI3_ACC_DP_OFF_XYZ_14_BIT_MASK              UINT16_C(0x3FFF)
*/
#define BMI3_ACC_DP_DOFFSET_X_MASK UINT16_C(0xFFFF)
#define BMI3_ACC_DP_DOFFSET_Y_MASK UINT16_C(0xFFFF)
#define BMI3_ACC_DP_DOFFSET_Z_MASK UINT16_C(0xFFFF)

#define BMI3_ACC_DP_DGAIN_X_MASK UINT16_C(0x00FF)
#define BMI3_ACC_DP_DGAIN_Y_MASK UINT16_C(0x00FF)
#define BMI3_ACC_DP_DGAIN_Z_MASK UINT16_C(0x00FF)

#define BMI3_GYR_DP_OFF_X_MASK UINT16_C(0x03FF)
#define BMI3_GYR_DP_DGAIN_X_MASK UINT16_C(0x007F)
#define BMI3_GYR_DP_OFF_Y_MASK UINT16_C(0x03FF)
#define BMI3_GYR_DP_DGAIN_Y_MASK UINT16_C(0x007F)
#define BMI3_GYR_DP_OFF_Z_MASK UINT16_C(0x03FF)
#define BMI3_GYR_DP_DGAIN_Z_MASK UINT16_C(0x007F)

/** @brief  IMU Accel range, in units of gravity acceleration (9.81m/s^2) */
typedef enum {
  IMU_ACCEL_RANGE_2G,     /**< +- 2Gs */
  IMU_ACCEL_RANGE_4G,     /**< +- 4Gs */
  IMU_ACCEL_RANGE_8G,     /**< +- 8Gs */
  IMU_ACCEL_RANGE_16G,    /**< +- 16Gs */
  NUM_IMU_ACCEL_RANGE     /**< Number of IMU Accelerometer ranges */
} IMUAccelRange;

/** @brief  IMU Gyro range in units of degrees */
typedef enum {
  IMU_GYRO_RANGE_125_DEG    /**< +- 125deg */,
  IMU_GYRO_RANGE_250_DEG    /**< +- 250deg */,
  IMU_GYRO_RANGE_500_DEG    /**< +- 500deg */,
  IMU_GYRO_RANGE_1000_DEG   /**< +- 1000deg */,
  IMU_GYRO_RANGE_2000_DEG   /**< +- 2000deg */,
  NUM_IMU_GYRO_RANGES
} IMUGyroRange;

/** @brief  BMI323 Registers */
typedef enum {
  /* General Registers */
  BMI323_REG_BANK_SEL = 0x7F,              /**< Register Bank Select */
  BMI323_REG_CHIP_ID = 0x00,               /**< Chip ID Register */
  BMI323_REG_ERROR_STATUS = 0x02,          /**< Error Status Register */
  BMI323_REG_RESET = 0x63,                 /**< Soft Reset Command Register */
  
  /* Accelerometer Registers */
  BMI323_REG_ACCEL_REG_ADDR = 0x03,        /**< Accelerometer Data Start Address */
  BMI323_REG_ACCEL_X_L = 0x12,             /**< Accelerometer X-axis Low Byte */
  BMI323_REG_ACCEL_X_H = 0x13,             /**< Accelerometer X-axis High Byte */
  BMI323_REG_ACCEL_Y_L = 0x14,             /**< Accelerometer Y-axis Low Byte */
  BMI323_REG_ACCEL_Y_H = 0x15,             /**< Accelerometer Y-axis High Byte */
  BMI323_REG_ACCEL_Z_L = 0x16,             /**< Accelerometer Z-axis Low Byte */
  BMI323_REG_ACCEL_Z_H = 0x17,             /**< Accelerometer Z-axis High Byte */
  BMI323_REG_ACC_CONF = 0x20,              /**< Accelerometer configuration values */

  /* Gyroscope Registers */
  BMI323_REG_GYRO_REG_ADDR = 0x06,         /**< Gyroscope Data Start Address */
  BMI323_REG_GYRO_X_L = 0x0C,              /**< Gyroscope X-axis Low Byte */
  BMI323_REG_GYRO_X_H = 0x0D,              /**< Gyroscope X-axis High Byte */
  BMI323_REG_GYRO_Y_L = 0x0E,              /**< Gyroscope Y-axis Low Byte */
  BMI323_REG_GYRO_Y_H = 0x0F,              /**< Gyroscope Y-axis High Byte */
  BMI323_REG_GYRO_Z_L = 0x10,              /**< Gyroscope Z-axis Low Byte */
  BMI323_REG_GYRO_Z_H = 0x11,              /**< Gyroscope Z-axis High Byte */
  BMI323_REG_GYRO_CONF = 0x21,             /**< Gyroscope configuration values */
  
  /* Control and Configuration Registers */
  BMI323_REG_CTRL_REG1 = 0x19,             /**< Control Register 1 */
  BMI323_REG_CTRL_REG2 = 0x1A,             /**< Control Register 2 */
  BMI323_REG_CTRL_REG3 = 0x1B,             /**< Control Register 3 */
  BMI323_REG_CTRL_REG4 = 0x1C,             /**< Control Register 4 */
  BMI323_REG_CTRL_REG5 = 0x1D,             /**< Control Register 5 */
  BMI323_REG_CTRL_REG6 = 0x1E,             /**< Control Register 6 */
  
  /* Power Management */
  BMI323_REG_PWR_CTRL = 0x6C,              /**< Power Control Register */
  BMI323_REG_PWR_CTRL2 = 0x6D,             /**< Power Control 2 Register */
  BMI323_REG_PWR_CTRL3 = 0x6E,             /**< Power Control 3 Register */
  BMI323_REG_PWR_CTRL4 = 0x6F,             /**< Power Control 4 Register */
  
  /* Interrupt Registers */
  BMI323_REG_INT_CONFIG = 0x50,            /**< Interrupt Configuration Register */
  BMI323_REG_INT_STATUS = 0x51,            /**< Interrupt Status Register */
  BMI323_REG_INT_EN_1 = 0x52,              /**< Interrupt Enable 1 Register */
  BMI323_REG_INT_EN_2 = 0x53,              /**< Interrupt Enable 2 Register */
  
  /* Status Registers */
  BMI323_REG_STATUS_REG = 0x1E,            /**< Status Register */
  BMI323_REG_FIFO_STATUS = 0x24,           /**< FIFO Status Register */
  
  /* Temperature Sensor Registers */
  BMI323_REG_TEMP_SENSOR = 0x22,           /**< Temperature Data Register */
  
  /* Calibration and Error Handling */
  BMI323_REG_CALIBRATION_REG = 0x24,       /**< Calibration Register */
  BMI323_REG_ERROR_REG = 0x2B,             /**< Error Register */
  
  /* Other Registers */
  BMI323_REG_CMD = 0x7E                    /**< Command Register (often used for resets and other operations) */
} Bmi323Registers;

/** @brief  IMU Accel Gain and Offset Registers */
typedef enum { ACC_DP_OFF_X = 0x60, ACC_DP_DGAIN_X = 0x61, ACC_DP_OFF_Y = 0x62, ACC_DP_DGAIN_Y = 0x63, ACC_DP_OFF_Z = 0x64, ACC_DP_DGAIN_Z = 0x65 } accel_go_registers;

/** @brief  IMU Accel Gain and Offset Registers */
typedef enum { GYR_DP_OFF_X = 0x66, GYR_DP_DGAIN_X = 0x67, GYR_DP_OFF_Y = 0x68, GYR_DP_DGAIN_Y = 0x69, GYR_DP_OFF_Z = 0x6A, GYR_DP_DGAIN_Z = 0x6B } gyro_go_registers;

typedef struct {
  uint16_t accel_offset_x;
  uint16_t accel_offset_y;
  uint16_t accel_offset_z;
  uint8_t accel_gain_x;
  uint8_t accel_gain_y;
  uint8_t accel_gain_z;
} AccelGainOffsetValues;

typedef struct {
  uint16_t gyro_offset_x;
  uint16_t gyro_offset_y;
  uint16_t gyro_offset_z;
  uint8_t gyro_gain_x;
  uint8_t gyro_gain_y;
  uint8_t gyro_gain_z;
} GyroGainOffsetValues;

typedef struct {
  int16_t x;
  int16_t y;
  int16_t z;
} Axes;

typedef struct {
  SpiPort spi_port;
  SpiSettings spi_settings;
  GpioAddress int1;
  GpioAddress int2;
  IMUAccelRange accel_range;
  IMUGyroRange gyro_range;
} Bmi323Settings;

typedef struct {
  Bmi323Settings *settings;
  Axes accel;
  Axes gyro;
  AccelGainOffsetValues accel_go_values;
  GyroGainOffsetValues gyro_go_values;
} Bmi323Storage;

StatusCode bmi323_init(Bmi323Storage *storage);

StatusCode bmi323_update();

/** @} */
