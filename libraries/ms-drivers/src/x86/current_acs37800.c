/************************************************************************************************
 * @file   current_acs37800.c
 *
 * @brief  x86 mock driver for the ACS37800 current sensing driver
 *
 * @date   2025-01-28
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>

/* Inter-component Headers */
#include "log.h"
#include "status.h"

/* Intra-component Headers */
#include "current_acs37800.h"
#include "current_acs37800_defs.h"
#define ACS37800_NUM_REGISTERS 0x30

static ACS37800Storage *s_storage = NULL;
static uint32_t s_registers[ACS37800_NUM_REGISTERS] = { 0 };

StatusCode acs37800_init(ACS37800Storage *storage, I2CPort i2c_port, I2CAddress i2c_address) {
  if (storage == NULL || i2c_address > 127) {
    return STATUS_CODE_INVALID_ARGS;
  }

  storage->i2c_port = i2c_port;
  storage->i2c_address = i2c_address;
  s_storage = storage;

  // Clear all registers
  for (int i = 0; i < ACS37800_NUM_REGISTERS; i++) {
    s_registers[i] = 0;
  }

  return STATUS_CODE_OK;
}

StatusCode acs37800_get_register(ACS37800Storage *storage, ACS37800_Registers reg, uint32_t *out_raw) {
  if (storage == NULL || out_raw == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  *out_raw = s_registers[reg];
  return STATUS_CODE_OK;
}

StatusCode acs37800_get_current(ACS37800Storage *storage, float *out_current_amps) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint32_t raw_data;

  StatusCode status = acs37800_get_register(storage, ACS37800_REG_VCODES_ICODES, &raw_data);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  // 16 bits upper
  int16_t current_raw = (int16_t)((raw_data >> 16) & 0xFFFF);
  *out_current_amps = (float)(current_raw)*CURRENT_SCALE;

  return STATUS_CODE_OK;
}

StatusCode acs37800_get_voltage(ACS37800Storage *storage, float *out_voltage_mV) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint32_t raw_data;

  StatusCode status = acs37800_get_register(storage, ACS37800_REG_VCODES_ICODES, &raw_data);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  // the voltage value is signed (16 bits lower)
  int16_t voltage_raw = (int16_t)(raw_data & 0xFFFF);
  *out_voltage_mV = (float)(voltage_raw)*VOLTAGE_SCALE;

  return STATUS_CODE_OK;
}

StatusCode acs37800_get_active_power(ACS37800Storage *storage, float *out_power_mW) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint32_t raw_data;

  StatusCode status = acs37800_get_register(storage, ACS37800_REG_PACTIVE_PIMAGE, &raw_data);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  // active power signed 16bit lower
  int16_t power_raw = (int16_t)(raw_data & 0xFFFF);
  *out_power_mW = (float)(power_raw)*POWER_SCALE;

  return STATUS_CODE_OK;
}

StatusCode acs37800_get_overcurrent_flag(ACS37800Storage *storage, bool *overcurrent_flag) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint32_t raw_data;

  StatusCode status = acs37800_get_register(storage, ACS37800_REG_STATUS, &raw_data);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  uint32_t converted_data = raw_data & ACS37800_MASK_FAULTOUT;

  if (converted_data) {
    *overcurrent_flag = true;
  } else {
    *overcurrent_flag = false;
  }

  return STATUS_CODE_OK;
}

StatusCode acs37800_reset_overcurrent_flag(ACS37800Storage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  // In ARM writes to I2C, instead just clear the flag bits
  s_registers[ACS37800_REG_STATUS] &= ~ACS37800_MASK_FAULTOUT;
  return STATUS_CODE_OK;
}

StatusCode acs37800_get_overvoltage_flag(ACS37800Storage *storage, bool *overvoltage_flag) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint32_t raw_data;

  StatusCode status = acs37800_get_register(storage, ACS37800_REG_STATUS, &raw_data);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  uint32_t converted_data = raw_data & ACS37800_MASK_OVERVOLTAGE;

  if (converted_data) {
    *overvoltage_flag = true;
  } else {
    *overvoltage_flag = false;
  }

  return STATUS_CODE_OK;
}

StatusCode acs37800_get_undervoltage_flag(ACS37800Storage *storage, bool *undervoltage_flag) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint32_t raw_data;

  StatusCode status = acs37800_get_register(storage, ACS37800_REG_STATUS, &raw_data);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  uint32_t converted_data = raw_data & ACS37800_MASK_UNDERVOLTAGE;

  if (converted_data) {
    *undervoltage_flag = true;
  } else {
    *undervoltage_flag = false;
  }

  return STATUS_CODE_OK;
}

/* Setters */

ACS37800Storage *acs37800_get_storage(void) {
  return s_storage;
}

void acs37800_set_register(ACS37800_Registers reg, uint32_t value) {
  s_registers[reg] = value;
}

void acs37800_set_current(float current_amps) {
  // Encode into upper 16 bits of VCODES_ICODES register
  int16_t current_raw = (int16_t)(current_amps / CURRENT_SCALE);
  // Clear upper 16 bits
  s_registers[ACS37800_REG_VCODES_ICODES] &= 0x0000FFFF;
  s_registers[ACS37800_REG_VCODES_ICODES] |= ((uint32_t)current_raw << 16);
}

void acs37800_set_voltage(float voltage_mV) {
  // Encode into lower 16 bits of VCODES_ICODES register
  int16_t voltage_raw = (int16_t)(voltage_mV / VOLTAGE_SCALE);
  // Clear lower 16 bits
  s_registers[ACS37800_REG_VCODES_ICODES] &= 0xFFFF0000;
  s_registers[ACS37800_REG_VCODES_ICODES] |= ((uint32_t)voltage_raw & 0xFFFF);
}

void acs37800_set_power(float power_mW) {
  // Encode into lower 16 bits of PACTIVE_PIMAGE register
  int16_t power_raw = (int16_t)(power_mW / POWER_SCALE);
  // Clear lower 16 bits
  s_registers[ACS37800_REG_PACTIVE_PIMAGE] &= 0xFFFF0000;
  s_registers[ACS37800_REG_PACTIVE_PIMAGE] |= ((uint32_t)power_raw & 0xFFFF);
}

void acs37800_set_overcurrent_flag(bool flag) {
  if (flag) {
    s_registers[ACS37800_REG_STATUS] |= ACS37800_MASK_FAULTOUT;
  } else {
    s_registers[ACS37800_REG_STATUS] &= ~ACS37800_MASK_FAULTOUT;
  }
}

void acs37800_set_overvoltage_flag(bool flag) {
  if (flag) {
    s_registers[ACS37800_REG_STATUS] |= ACS37800_MASK_OVERVOLTAGE;
  } else {
    s_registers[ACS37800_REG_STATUS] &= ~ACS37800_MASK_OVERVOLTAGE;
  }
}

void acs37800_set_undervoltage_flag(bool flag) {
  if (flag) {
    s_registers[ACS37800_REG_STATUS] |= ACS37800_MASK_UNDERVOLTAGE;
  } else {
    s_registers[ACS37800_REG_STATUS] &= ~ACS37800_MASK_UNDERVOLTAGE;
  }
}
