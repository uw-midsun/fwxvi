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
    

    return STATUS_CODE_OK;
}

StatusCode acs37800_get_current(ACS37800_Storage *storage, float *out_current_amps) {
    return STATUS_CODE_OK;
}

StatusCode acs37800_get_voltage(ACS37800_Storage *storage, float *out_voltage_mV) {
    return STATUS_CODE_OK;
}

StatusCode acs37800_get_power(ACS37800_Storage *storage, float *out_power_watts) {
    return STATUS_CODE_OK;
}

StatusCode acs37800_get_register(ACS37800_Storage *storage, ACS37800_Registers reg, uint32_t *out_raw) {
    return STATUS_CODE_OK;
}
