#pragma once

/************************************************************************************************
 * @file    current_acs37800_defs.h
 *
 * @brief   Current Sensor ACS37800 Defs
 *
 * @date    2025-07-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup current_acs37800_defs
 * @brief    current_acs37800_defs Firmware
 * @{
 */

// Table value formula, NEED the Rsense and Riso resistor values for line voltage calculation
#define VOLTAGE_SCALE ((0.84 * 1.19) / 32768.0)

#define CURRENT_SCALE ((0.84 * 1.19) / 32768.0)
#define POWER_SCALE ((0.704 * 1.42) / 32768.0)
#define ACS37800_MASK_FAULTOUT 0x02      // Bit 1
#define ACS37800_MASK_OVERVOLTAGE 0x08   // Bit 3
#define ACS37800_MASK_UNDERVOLTAGE 0x10  // Bit 4

// register map is on pg.38
// https://www.allegromicro.com/-/media/files/datasheets/acs37800-datasheet.ashx
typedef enum {
  ACS37800_REG_VRMS_IRMS = 0x20,          // Voltage and Current RMS
  ACS37800_REG_VCODES_ICODES = 0x2A,      // Voltage and Current Instantaneous
  ACS37800_REG_PINSTANT = 0x2C,           // Power Instantaneous
  ACS37800_REG_PACTIVE_PIMAGE = 0x21,     // Active & Reactive Power Output
  ACS37800_REG_PAPPARENT_PFACTOR = 0x22,  // Apparent power & Power factor output
  ACS37800_REG_NUMPTSOUT = 0x25,          // number of points for RMS calculation
  ACS37800_REG_VI_RMSAVGONESEC = 0x26,    // Voltage & Current average RMS (seconds)
  ACS37800_REG_VI_RMSAVGONEMIN = 0x27,    // Voltage & Current average RMS (minutes)
  ACS37800_PACTAVGONESEC = 0x28,          // Power average RMS (seconds)
  ACS37800_REG_PACTAVGONEMIN = 0x29,      // Power average RMS (minutes)
  ACS37800_REG_STATUS = 0x2D              // ZEROCROSSOUT, OVERVOLTAGE, UNDERVOLTAGE, FAULTOUT, FAULTLATCHED
} ACS37800_Registers;

/** @} */
