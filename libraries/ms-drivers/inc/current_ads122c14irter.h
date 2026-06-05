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


 // this is a 24 bit guy

//# define I2CAddress ADC_ADDRESS = 0x40; /* A0 and A1 are both wired to GND*/


typedef struct {
  I2CPort i2c_port;
  I2CAddress i2c_address;
} ADS122Storage;

typedef enum : uint8_t{
  ADS122_REG_DEVICE_ID = 0b00000000,
  ADS122_REG_REVISION_ID = 0b00000001,
  ADS122_REG_STATUS_MSB = 0b00000010,
  ADS122_REG_STATUS_LSB = 0b00000011,
  ADS122_REG_CONVERSION_CTRL = 0b00000100,
  ADS122_REG_DEVICE_CFG = 0b00000101,
  ADS122_REG_DATA_RATE_CFG = 0b00000110,
  ADS122_REG_MUX_CFG = 0b00000111,
  ADS122_REG_GAIN_CFG = 0b00001000,
  ADS122_REG_REFERENCE_CFG = 0b00001001,
  ADS122_REG_DIGITAL_CFG = 0b00001010,
  ADS122_REG_GPIO_CFG = 0b00001011,
  ADS122_REG_GPIO_DATA_OUTPUT = 0b00001100,
  ADS122_REG_IDAC_MAG_CFG = 0b00001101,
  ADS122_REG_IDAC_MUX_CFG = 0b00001110,
  ADS122_REG_REG_MAP_CRC = 0b00001111,
} ADS122C14ITER_Register;

#define ADS122_NUM_REGS = 16U;

typedef enum : uint8_t{
  ADS122_WRITE_COMMAND = 0b10000000,
  ADS122_READ_COMMAND = 0b01000000,
  ADS122_READ_CONVERSION_COMMAND = 0b00000000,
} ADS122C14ITER_Command;

_Static_assert(sizeof(ADS122C14ITER_Register) == 1U );
_Static_assert(sizeof(ADS122C14ITER_Command) == 1U );









/* Registers*/
/**
 * Device ID
 * STATUS_MSB
 * Device config regs
 * CONVERSION_CTRL
 */

/* What timing mode? <- timing requirements*/