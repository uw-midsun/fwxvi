/************************************************************************************************
 * @file   current_acs37800.c
 *
 * @brief  Source file to implement the ACS37800 current sensing driver
 *
 * @date   2025-07-30
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "current_acs37800.h"

// initialize the storage object
StatusCode acs37800_init(ACS37800_Storage *storage, I2CPort *i2c_port, I2CAddress *i2c_address) {
  if (storage == NULL || i2c_address == NULL || i2c_port == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  // i2c peripherals
  storage->i2c_port = i2c_port;
  storage->i2c_address = i2c_address;

  // conversion scaling factors
  storage->current_per_amp = CURRENT_SCALE;
  storage->voltage_per_volt = VOLTAGE_SCALE;
  storage->power_per_watt = POWER_SCALE;

  return STATUS_CODE_OK;
}

StatusCode acs37800_get_register(ACS37800_Storage *storage, ACS37800_Registers reg, uint32_t *out_raw) {
  if (storage == NULL || reg == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint8_t reg_addr = (uint8_t)reg;
  uint8_t rx_buff[4];

  StatusCode status = i2c_read_reg(storage->i2c_port, storage->i2c_address, reg_addr, rx_buff, sizeof(rx_buff));

  if (status != STATUS_CODE_OK) {
    return status;
  }

  *out_raw = ((uint32_t)rx_buff[0] << 24) | ((uint32_t)rx_buff[1] << 16) | ((uint32_t)rx_buff[2] << 8) | ((uint32_t)rx_buff[3]);

  return STATUS_CODE_OK;
}

StatusCode acs37800_get_current(ACS37800_Storage *storage, float *out_current_amps) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint32_t raw_data;

  StatusCode status = acs37800_get_register(storage, ACS37800_REG_VCODES_ICODES, raw_data);

  // the current value is signed (16 bits upper)
  int16_t current_raw = (int16_t)((raw_data >> 16) & 0xFFFF);

  *out_current_amps = (float)(current_raw)*storage->current_per_amp;

  return STATUS_CODE_OK;
}

StatusCode acs37800_get_voltage(ACS37800_Storage *storage, float *out_voltage_mV) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint32_t raw_data;

  StatusCode status = acs37800_get_register(storage, ACS37800_REG_VCODES_ICODES, raw_data);

  // the voltage value is signed (16 bits lower)
  int16_t voltage_raw = (int16_t)(raw_data & 0xFFFF);

  *out_voltage_mV = (float)(voltage_raw)*storage->voltage_per_volt;

  return STATUS_CODE_OK;
}

StatusCode acs37800_get_active_power(ACS37800_Storage *storage, float *out_power_mW) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint32_t raw_data;

  StatusCode status = acs37800_get_register(storage, ACS37800_REG_PACTIVE_PIMAGE, raw_data);

  // the active power value is signed (16 bits lower)
  int16_t voltage_raw = (int16_t)(raw_data & 0xFFFF);

  *out_power_mW = (float)(voltage_raw)*storage->power_per_watt;

  return STATUS_CODE_OK;
}
