#pragma once

/************************************************************************************************
 * @file   bms_hw_defs.h
 *
 * @brief  Header file for BMS hardware definitions
 *
 * @date   2025-01-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "adc.h"
#include "bms.h"
#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"
#include "gpio.h"
#include "log.h"
#include "pwm.h"

/* Intra-component Headers */

/**
 * @defgroup bms_carrier
 * @brief    bms_carrier Firmware
 * @{
 */

#define BMS_FAN_PERIOD 40                // Period in ms. Frequency = 25,000 Hz
#define BMS_FAN_TEMP_UPPER_THRESHOLD 50  // Threshold before fan is full strength
#define BMS_FAN_TEMP_LOWER_THRESHOLD 40  // Threshold before fan is full strength

void bms_run_fan(void);
void bms_fan_init(BmsStorage *storage);
/** @} */
