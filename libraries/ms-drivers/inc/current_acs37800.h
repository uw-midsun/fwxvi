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

/* Inter-component Headers */
#include <stdint.h>
#include "i2c.h"
#include "gpio.h"
#include "current_acs37800_defs.h"

/* Intra-component Headers */

/**
 * @defgroup ACS37800 Current Sensing
 * @brief    ACS37800 Current Sensing library
 * @{
 */

typedef struct {
    I2CPort i2c_port;
    I2CAddress i2c_address;

    float current_per_amp;
    float voltage_per_volt;
    float power_per_watt;

    // over / under current
    GpioAddress dio0; // zero-crossing
    GpioAddress dio1; // overcurrent fault
} ACS37800_Storage;



/* @brief Initialize the ACS37800 driver
 * @param storage - a pointer to the ACS37800 struct, to be initialized
 * @param i2c_port - I2C port peripheral 
 * @param i2c_address - I2C address peripheral 
 * @return STATUS_CODE_OK on success
 */
StatusCode acs37800_init(ACS37800_Storage *storage, I2CPort *i2c_port, I2CAddress *i2c_address);

 /* @brief Gets the instantaneous current in amps
 * @param storage - pointer to already initialized ACS37800 struct
 * @param out_current - current in amps
 * @return STATUS_CODE_OK on success
 */
StatusCode acs37800_get_current(ACS37800_Storage *storage, float *out_current);

/* @brief Gets the instantaneous voltage in volts
 * @param storage - pointer to already initialized ACS37800 struct
 * @param out_voltage - voltage in volts
 * @return STATUS_CODE_OK on success
 */
StatusCode acs37800_get_voltage(ACS37800_Storage *storage, float *out_voltage);

/* @brief Gets the instantaneous power in watts
 * @param storage - pointer to already initialized ACS37800 struct
 * @param out_power - power in watts
 * @return STATUS_CODE_OK on success
 */
StatusCode acs37800_get_power(ACS37800_Storage *storage, float *out_power);

/* @brief Gets the 16 bit value from the ACS37800 volatile register
 * @param storage - pointer to already initialized ACS37800 struct
 * @param reg - register address
 * @param out - 16 bit value at that register
 * @return STATUS_CODE_OK on success
 */
StatusCode acs37800_get_register(ACS37800_Storage *storage, ACS37800_Registers reg, uint16_t *out);

/** @} */
