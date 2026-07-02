#pragma once

/************************************************************************************************
 * @file    tcixtma1.h
 *
 * @brief   Header file to implement the hydrogen sensing from the TCIXTMA1 sensor
 *
 * @date    2026-06-07
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/
/* Standard library Headers */
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */
#include "gpio.h"
#include "i2c.h"
#include "status.h"

/* Intra-component Headers */
#include "tcixtma1_defs.h"

/**
 * @defgroup TCIXTMA1 Hydrogen Sensing
 * @brief    TCIXTMA1 Hydrogen Sensing library
 * @{
 */

typedef struct {
  I2CPort i2c_port;
  I2CAddress i2c_address;
} TCIXTMA1Storage;

/**
 * @brief Initialize the TCIXTMA1 driver
 * @param storage - a pointer to the TCIXTMA1 struct, to be initialized
 * @param i2c_port - I2C port peripheral
 * @param i2c_address - I2C address peripheral
 * @return STATUS_CODE_OK on success
 */
StatusCode tcixtma1_init(TCIXTMA1Storage *storage, I2CPort i2c_port, I2CAddress i2c_address);

/**
 * @brief Gets the instantaneous current in Amps
 * @param storage - pointer to already initialized TCIXTMA1 struct
 * @param out_hydrogen_con - hydrogen concentration
 * @return STATUS_CODE_OK on success
 */
StatusCode tcixtma1_get_hydrogen_concentration(TCIXTMA1Storage *storage, float *out_hydrogen_con);

/** @} */
