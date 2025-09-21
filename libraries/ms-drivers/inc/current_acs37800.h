#pragma once

/************************************************************************************************
 * @file   current_sense_acs37800.h
 *
 * @brief  Header file to implement the current sensing from the ACS37800 sensor
 *
 * @date   2025-07-29
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#include "gpio.h"
#include "i2c.h"
#include "status.h"

/* Intra-component Headers */
#include "current_acs37800_defs.h"

/**
 * @defgroup ACS37800 Current Sensing
 * @brief    ACS37800 Current Sensing library
 * @{
 */

typedef struct {
  I2CPort i2c_port;
  I2CAddress i2c_address;

  // convert raw data to valid units
  float current_per_amp;
  float voltage_per_volt;
  float power_per_watt;
} ACS37800_Storage;

/** @brief Initialize the ACS37800 driver
 * @param storage - a pointer to the ACS37800 struct, to be initialized
 * @param i2c_port - I2C port peripheral
 * @param i2c_address - I2C address peripheral
 * @return STATUS_CODE_OK on success
 */
StatusCode acs37800_init(ACS37800_Storage *storage, I2CPort i2c_port, I2CAddress i2c_address);

/** @brief Gets the instantaneous current in amps
 * @param storage - pointer to already initialized ACS37800 struct
 * @param out_current_amps - current in amps
 * @return STATUS_CODE_OK on success
 */
StatusCode acs37800_get_current(ACS37800_Storage *storage, float *out_current_amps);

/** @brief Gets the instantaneous voltage in volts
 * @param storage - pointer to already initialized ACS37800 struct
 * @param out_voltage_mV - voltage in milivolts
 * @return STATUS_CODE_OK on success
 */
StatusCode acs37800_get_voltage(ACS37800_Storage *storage, float *out_voltage_mV);

/** @brief Gets the instantaneous power in milliwatts
 * @param storage - pointer to already initialized ACS37800 struct
 * @param out_power - power in milliwatts
 * @return STATUS_CODE_OK on success
 */
StatusCode acs37800_get_active_power(ACS37800_Storage *storage, float *out_power_mW);

/** @brief Gets if the ACS37800 unit detects over-current (latched)
 * @param storage - pointer to already initialized ACS37800 struct
 * @param overcurrent_flag - boolean to state if over-current happened (true -> over-current active)
 * @return STATUS_CODE_OK on success
 */
StatusCode acs37800_get_overcurrent_flag(ACS37800_Storage *storage, bool *overcurrent_flag);

/** @brief Resets the latched overcurrent fault, needs to write 1 to 0x2D register
 * @param storage - pointer to already initialized ACS37800 struct
 * @return STATUS_CODE_OK on success
 */
StatusCode acs37800_reset_overcurrent_flag(ACS37800_Storage *storage);

/** @brief Gets if the ACS37800 unit detects over-voltage
 * @param storage - pointer to already initialized ACS37800 struct
 * @param overcurrent_flag - boolean to state if over-voltage happened (true -> over-voltage active)
 * @return STATUS_CODE_OK on success
 */
StatusCode acs37800_get_overvoltage_flag(ACS37800_Storage *storage, bool *overvoltage_flag);

/** @brief Gets if the ACS37800 unit detects under-voltage
 * @param storage - pointer to already initialized ACS37800 struct
 * @param overcurrent_flag - boolean to state if under-voltage happened (true -> under-voltage active)
 * @return STATUS_CODE_OK on success
 */
StatusCode acs37800_get_undervoltage_flag(ACS37800_Storage *storage, bool *undervoltage_flag);

/** @brief Gets the 16 bit value from the ACS37800 volatile register
 * @param storage - pointer to already initialized ACS37800 struct
 * @param reg - register address
 * @param out - 32 bit value at that register
 * @return STATUS_CODE_OK on success
 */
StatusCode acs37800_get_register(ACS37800_Storage *storage, ACS37800_Registers reg, uint32_t *out_raw);

/** @} */
