/************************************************************************************************
 * @file   current_ads122c14irter.c
 *
 * @brief  x86 stub implementation for the ADS122 current-sense driver
 *
 * @date   2026-07-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <string.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "current_ads122c14irter.h"

static uint8_t s_registers[16];
static uint8_t s_conversion_data[5];

StatusCode ads122_start_conversion(ADS122Storage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  /* Set START pin to 1 -> ARM code reads then writes to the register*/
  uint8_t conversion_ctrl = 0x00;
  conversion_ctrl = s_registers[ADS122_REG_CONVERSION_CTRL];
  conversion_ctrl |= (1 << 1);
  s_registers[ADS122_REG_CONVERSION_CTRL] = conversion_ctrl;

  return STATUS_CODE_OK;
}

StatusCode ads122_change_MUX(ADS122Storage *storage, uint8_t MUX_CFG) {
  // (void)MUX_CFG;
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  /* ARM code writes the new MUX_CFG to the ADS122_REG_MUX_CFG register*/
  s_registers[ADS122_REG_MUX_CFG] = MUX_CFG;

  return STATUS_CODE_OK;
}

StatusCode ads122_configure(ADS122Storage *storage, uint8_t register_map[]) {
  if (storage == NULL || register_map == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  for (int i = 0; i < 16; i++) {
    s_registers[i] = register_map[i];
  }

  uint8_t reset_status_msb = 0xC0;
  s_registers[ADS122_REG_STATUS_MSB] = reset_status_msb;

  return STATUS_CODE_OK;
}

StatusCode ads122_init(ADS122Storage *storage, I2CPort i2c_port_storage, I2CAddress i2c_address_storage, uint8_t register_map[], I2CSettings *i2c_settings_storage) {
  if (storage == NULL || i2c_settings_storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  storage->i2c_port = i2c_port_storage;
  storage->i2c_address = i2c_address_storage;
  storage->i2c_settings = *i2c_settings_storage;

  return ads122_configure(storage, register_map);
}

StatusCode ads122_get_conversion_data(ADS122Storage *storage, uint8_t rx_data[]) {
  if (storage == NULL || rx_data == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  memset(s_conversion_data, *rx_data, 5U);
  return STATUS_CODE_OK;
}

StatusCode ads122_reset(ADS122Storage *storage) {
  return STATUS_CODE_OK;
}
