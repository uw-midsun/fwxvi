#pragma once

/************************************************************************************************
 * @file   current_ads122c14iter.h
 *
 * @brief  Header file to implement the current sensing from the ADS122C14ITER ADC
 *
 * @date   2026-06-03
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
 * @defgroup ADS122C14ITER Current Sensing
 * @brief    ADS122C14ITER Current Sensing library
 * @{
 */


 typedef struct {
  I2CPort i2c_port;
  I2CAddress i2c_address;
} ADS122C14ITERStorage;





/* Registers*/
/**
 * Device ID
 * STATUS_MSB
 * Device config regs
 * CONVERSION_CTRL
 */

/* What timing mode? <- timing requirements*/