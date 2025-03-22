#pragma once

/************************************************************************************************
 * @file   fann.h
 *
 * @brief  Header file for BMS fans
 *
 * @date   2025-02-06
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "gpio.h"
#include "log.h"

/* Intra-component Headers */
#include "bms_carrier.h"
#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"

/**
 * @defgroup bms_carrier
 * @brief    bms_carrier Firmware
 * @{
 */

/**
 * @brief   Fan settings
 */
struct FanSettings {
  GpioAddress fan1_sense;   /**< Fan 1 sense */
  GpioAddress fan2_sense;   /**< Fan 2 sense */
  GpioAddress fan_pwm_ctrl; /**< Fan PWM control */
};

/** @brief  Period in ms. Frequency = 25,000 Hz */
#define BMS_FAN_PERIOD_MS 40U
/** @brief  Threshold before fan is full strength */
#define BMS_FAN_TEMP_UPPER_THRESHOLD 50U
/** @brief  Threshold before fan is full strength */
#define BMS_FAN_TEMP_LOWER_THRESHOLD 40U
/** @brief  Base fan duty cycle when minimum temperature threshold is passed */
#define BMS_FAN_BASE_DUTY_CYCLE 50U

/**
 * @brief   Initialize the BMS fans
 * @param   storage Pointer to the BMS storage
 * @return  STATUS_CODE_OK if state of charge initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode fans_init(BmsStorage *storage);

/**
 * @brief   Update the BMS fans output
 */
void update_fans(void);

/**
 * @brief   Calculate the fan duty cycle
 * @param   temperature Maximum temperature to calculate fan duty cycle
 * @return  Fan duty cycle from 0% - 100%
 */
uint8_t calculate_fan_dc(uint16_t temperature);

/** @} */
