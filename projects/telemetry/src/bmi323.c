/************************************************************************************************
 * @file   bmi323.c
 *
 * @brief  Source file for BMI323 driver
 *
 * @date   2026-02-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "log.h"
#include "status.h"
#include "system_can.h"

/* Intra-component Headers */
#include "bmi323.h"
#include "imu_hw_defs.h"

#define DUMMY_BYTE 0x00U

#define WRITE_MASK 0x7FU
#define READ_BIT 0x80U

/************************************************************************************************
 * Static Variable Declaration
 ************************************************************************************************/
static Bmi323Storage *imu_storage;

static const float s_accel_lsb_lookup[NUM_IMU_ACCEL_RANGES] = {
  [IMU_ACCEL_RANGE_2G] = 16384.0f,
  [IMU_ACCEL_RANGE_4G] = 8192.0f,
  [IMU_ACCEL_RANGE_8G] = 4096.0f,
  [IMU_ACCEL_RANGE_16G] = 2048.0f,
};

static const float s_gyro_lsb_lookup[NUM_IMU_GYRO_RANGES] = {
  [IMU_GYRO_RANGE_125_DEG] = 262.4f, [IMU_GYRO_RANGE_250_DEG] = 131.2f, [IMU_GYRO_RANGE_500_DEG] = 65.6f, [IMU_GYRO_RANGE_1000_DEG] = 32.8f, [IMU_GYRO_RANGE_2000_DEG] = 16.4f,
};

/************************************************************************************************
 * Private Function Declaration
 ************************************************************************************************/

static StatusCode s_get_register(Bmi323Registers reg, uint16_t *data);
static StatusCode s_get_multi_register(Bmi323Registers reg, uint16_t *data, uint8_t len);
static StatusCode s_set_register(Bmi323Registers reg, uint16_t data);
static StatusCode s_set_multi_register(Bmi323Registers reg, uint16_t *data, uint8_t len);

static StatusCode get_gyroscope_data(Axes *gyro);
static StatusCode get_accel_data(Axes *accel);

static StatusCode set_gyro_offset_gain(GyroGainOffsetValues *gyro_go_values);
static StatusCode set_accel_offset_gain(AccelGainOffsetValues *accel_go_values);

static void s_calculate_accel_offset();
static void s_calculate_gyro_offset();
static uint8_t s_get_chip_id();

/************************************************************************************************
 * Private Function Definition
 ************************************************************************************************/

static StatusCode s_get_register(Bmi323Registers reg, uint16_t *data) {
  uint8_t tx_buffer[2U];

  tx_buffer[0U] = READ_BIT | reg;
  tx_buffer[1U] = DUMMY_BYTE;

  return spi_exchange(imu_storage->settings->spi_port, tx_buffer, sizeof(tx_buffer), (uint8_t *)data, sizeof(uint16_t));
}

static StatusCode s_get_multi_register(Bmi323Registers reg, uint16_t *data, uint8_t len) {
  uint8_t tx_buffer[2U];

  tx_buffer[0U] = READ_BIT | reg;
  tx_buffer[1U] = DUMMY_BYTE;

  return spi_exchange(imu_storage->settings->spi_port, tx_buffer, sizeof(tx_buffer), (uint8_t *)data, sizeof(uint16_t) * len);
}

static StatusCode s_set_register(Bmi323Registers reg, uint16_t data) {
  uint8_t tx_buffer[3U];

  tx_buffer[0U] = reg & WRITE_MASK;
  tx_buffer[1U] = data & 0xFFU;
  tx_buffer[2U] = (data >> 8U) & 0xFFU;

  return spi_exchange(imu_storage->settings->spi_port, tx_buffer, sizeof(tx_buffer), NULL, 0);
}

static StatusCode s_set_multi_register(Bmi323Registers reg, uint16_t *data, uint8_t len) {
  uint16_t tx_buffer[BMI323_MAX_NUM_DATA] = { 0U };

  tx_buffer[0U] = WRITE_MASK & reg;

  /* Copy data */
  for (size_t i = 1; i < len + 1U; i++) {
    tx_buffer[i] = data[i - 1U];
  }

  return spi_exchange(imu_storage->settings->spi_port, (uint8_t *)tx_buffer, sizeof(tx_buffer), NULL, 0U);
}

static StatusCode get_gyroscope_data(Axes *gyro) {
  uint16_t data[3] = { 0 };

  StatusCode status = s_get_multi_register(BMI323_REG_GYRO_REG_ADDR, data, 3);

  if (status == STATUS_CODE_OK) {
    int16_t raw_x = (int16_t)(data[0]);
    int16_t raw_y = (int16_t)(data[1]);
    int16_t raw_z = (int16_t)(data[2]);

    gyro->x = (float)raw_x / s_gyro_lsb_lookup[imu_storage->settings->gyro_range];
    gyro->y = (float)raw_y / s_gyro_lsb_lookup[imu_storage->settings->gyro_range];
    gyro->z = (float)raw_z / s_gyro_lsb_lookup[imu_storage->settings->gyro_range];

    return STATUS_CODE_OK;
  }

  return status;
}

static StatusCode get_accel_data(Axes *accel) {
  uint16_t data[3] = { 0 };
  StatusCode status = s_get_multi_register(BMI323_REG_ACCEL_REG_ADDR, data, 3);

  if (status == STATUS_CODE_OK) {
    int16_t raw_x = (int16_t)(data[0]);
    int16_t raw_y = (int16_t)(data[1]);
    int16_t raw_z = (int16_t)(data[2]);

    accel->x = (float)raw_x / s_accel_lsb_lookup[imu_storage->settings->accel_range];
    accel->y = (float)raw_y / s_accel_lsb_lookup[imu_storage->settings->accel_range];
    accel->z = (float)raw_z / s_accel_lsb_lookup[imu_storage->settings->accel_range];
  }

  return status;
}

static StatusCode set_gyro_offset_gain(GyroGainOffsetValues *gyro_go_values) {
  uint16_t data[6U] = { 0U };

  uint16_t gyro_off_x, gyro_off_y, gyro_off_z;
  uint8_t gyro_gain_x, gyro_gain_y, gyro_gain_z;

  if (gyro_go_values == NULL) return BMI3_E_NULL_PTR;

  gyro_off_x = gyro_go_values->gyro_offset_x & BMI3_GYR_DP_OFF_X_MASK;
  gyro_off_y = gyro_go_values->gyro_offset_y & BMI3_GYR_DP_OFF_Y_MASK;
  gyro_off_z = gyro_go_values->gyro_offset_z & BMI3_GYR_DP_OFF_Z_MASK;

  gyro_gain_x = gyro_go_values->gyro_gain_x & BMI3_GYR_DP_DGAIN_X_MASK;
  gyro_gain_y = gyro_go_values->gyro_gain_y & BMI3_GYR_DP_DGAIN_Y_MASK;
  gyro_gain_z = gyro_go_values->gyro_gain_z & BMI3_GYR_DP_DGAIN_Z_MASK;

  data[0] = gyro_off_x;
  data[1] = (uint16_t)gyro_gain_x;

  data[2] = gyro_off_y;
  data[3] = (uint16_t)gyro_gain_y;

  data[4] = gyro_off_z;
  data[5] = (uint16_t)gyro_gain_z;

  StatusCode result = s_set_multi_register(GYR_DP_OFF_X, data, 6);

  return result;
}

static StatusCode set_accel_offset_gain(AccelGainOffsetValues *accel_go_values) {
  uint16_t data[6U] = { 0U };

  uint16_t accel_off_X, accel_off_y, accel_off_z;
  uint8_t accel_gain_x, accel_gain_y, accel_gain_z;

  if (accel_go_values == NULL) return BMI3_E_NULL_PTR;

  accel_off_X = accel_go_values->accel_offset_x & BMI3_ACC_DP_DOFFSET_X_MASK;
  accel_off_y = accel_go_values->accel_offset_y & BMI3_ACC_DP_DOFFSET_Y_MASK;
  accel_off_z = accel_go_values->accel_offset_z & BMI3_ACC_DP_DOFFSET_Z_MASK;

  accel_gain_x = accel_go_values->accel_gain_x & BMI3_ACC_DP_DGAIN_X_MASK;
  accel_gain_y = accel_go_values->accel_gain_y & BMI3_ACC_DP_DGAIN_Y_MASK;
  accel_gain_z = accel_go_values->accel_gain_z & BMI3_ACC_DP_DGAIN_Z_MASK;

  data[0] = accel_off_X;
  data[1] = (uint16_t)accel_gain_x;

  data[2] = accel_off_y;
  data[3] = (uint16_t)accel_gain_y;

  data[4] = accel_off_z;
  data[5] = (uint16_t)accel_gain_z;

  StatusCode result = s_set_multi_register(ACC_DP_OFF_X, data, 6);

  return result;
}

static void s_calculate_accel_offset() {
  int16_t accel_x_off = 0, accel_y_off = 0, accel_z_off = 0;

  for (int i = 0; i < 100; ++i) {
    get_accel_data(&imu_storage->accel);

    accel_x_off += imu_storage->accel.x;
    accel_y_off += imu_storage->accel.y;
    accel_z_off += imu_storage->accel.z;
  }

  accel_x_off /= 100;
  accel_y_off /= 100;
  accel_z_off /= 100;
  accel_x_off = -accel_x_off;
  accel_y_off = -accel_y_off;
  accel_z_off = -accel_z_off;

  imu_storage->accel_go_values.accel_offset_x = (uint16_t)accel_x_off;
  imu_storage->accel_go_values.accel_offset_y = (uint16_t)accel_y_off;
  imu_storage->accel_go_values.accel_offset_z = (uint16_t)accel_z_off;
}

static void s_calculate_gyro_offset() {
  int16_t gyr_x_off = 0, gyr_y_off = 0, gyr_z_off = 0;

  for (int i = 0; i < 100; ++i) {
    get_gyroscope_data(&imu_storage->gyro);

    gyr_x_off += imu_storage->gyro.x;
    gyr_y_off += imu_storage->gyro.y;
    gyr_z_off += imu_storage->gyro.z;
  }

  gyr_x_off /= 100;
  gyr_y_off /= 100;
  gyr_z_off /= 100;
  gyr_x_off = -gyr_x_off;
  gyr_y_off = -gyr_y_off;
  gyr_z_off = -gyr_z_off;

  imu_storage->gyro_go_values.gyro_offset_x = (uint16_t)gyr_x_off;
  imu_storage->gyro_go_values.gyro_offset_y = (uint16_t)gyr_y_off;
  imu_storage->gyro_go_values.gyro_offset_z = (uint16_t)gyr_z_off;
}

static uint8_t s_get_chip_id() {
  uint16_t chip_id = 0U;

  s_get_register(BMI323_REG_CHIP_ID, &chip_id);

  return (chip_id & 0xFFU);
}

/************************************************************************************************
 * Public Function Definition
 ************************************************************************************************/

StatusCode bmi323_init(Bmi323Storage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  imu_storage = storage;

  s_set_register(BMI323_REG_CMD, 0xDEAF);  // soft reset

  uint16_t data = DUMMY_BYTE;

  /* Write dummy byte to initialize SPI as per datasheet */
  s_get_register(BMI323_REG_CHIP_ID, &data);

  /* BMI323 Startup time */
  delay_ms(10U);

  /* Poll the chip ID */
  for (uint8_t i = 0U; i < 10U; i++) {
    if (data == BMI323_CHIP_ID) {
      break;
    }
    data = s_get_chip_id();
    delay_ms(10U);
  }

  if (data != BMI323_CHIP_ID) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  /* CONFIGURE ACCELEROMETER
   * Set range
   * Set ODR
   * Enable (in high performance mode)
   */
  uint16_t acc_conf;
  s_get_register(BMI323_REG_ACC_CONF, &acc_conf);

  acc_conf &= ~(0b111 << 4);
  acc_conf |= (imu_storage->settings->accel_range << 4);
  acc_conf &= ~(0b1111);
  acc_conf |= 0b1110;
  acc_conf &= ~(0b111 << 12);
  acc_conf |= (0x7 << 12);

  s_set_register(BMI323_REG_ACC_CONF, (uint16_t)acc_conf);

  /* CONFIGURE GYROSCOPE
   * Set range
   * Set ODR
   * Enable (in high performance mode)
   */
  uint16_t gyr_conf;
  s_get_register(BMI323_REG_GYRO_CONF, &gyr_conf);

  gyr_conf &= ~(0b111 << 4);
  gyr_conf |= (imu_storage->settings->gyro_range << 4);
  gyr_conf &= ~(0b1111);
  gyr_conf |= 0b1110;
  gyr_conf &= ~(0b111 << 12);
  gyr_conf |= (0x7 << 12);

  s_set_register(BMI323_REG_GYRO_CONF, (uint16_t)gyr_conf);

  uint16_t activate_config;
  s_get_register(BMI323_REG_CHIP_ID, &activate_config);

  s_calculate_accel_offset();
  set_accel_offset_gain(&imu_storage->accel_go_values);

  s_calculate_gyro_offset();
  set_gyro_offset_gain(&imu_storage->gyro_go_values);

  // gyro_crt_calibration();

  return STATUS_CODE_OK;
}

StatusCode bmi323_update(Bmi323Storage *storage) {
  /* Update GYRO/ACCEL readings */

  get_accel_data(&storage->accel);
  get_gyroscope_data(&storage->gyro);

  return STATUS_CODE_OK;
}
