/************************************************************************************************
 * @file   i2c.c
 *
 * @brief  I2C Library Source Code
 *
 * @date   2024-11-29
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "i2c.h"

StatusCode i2c_init(I2CPort i2c, const I2CSettings *settings) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode i2c_read(I2CPort i2c, I2CAddress addr, uint8_t *rx_data, size_t rx_len) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode i2c_write(I2CPort i2c, I2CAddress addr, uint8_t *tx_data, size_t tx_len) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode i2c_read_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *rx_data, size_t rx_len) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode i2c_write_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *tx_data, size_t tx_len) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode i2c_set_data(I2CPort i2c, uint8_t *data, size_t len) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode i2c_get_data(I2CPort i2c, uint8_t *data, size_t len) {
  return STATUS_CODE_UNIMPLEMENTED;
}
