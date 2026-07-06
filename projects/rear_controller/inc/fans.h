#pragma once

/************************************************************************************************
 * @file   fans.h
 *
 * @brief  Fans header file
 *
 * @date   2025-09-20
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "rear_controller.h"

/**
 * @defgroup Rear_Controller
 * @brief    Rear Controller Board Firmware
 * @{
 */

/* PWM fans idle at this duty and ramp linearly to full over the temp band below */
#define REAR_CONTROLLER_FAN_IDLE_DUTY_PCT 10U
#define REAR_CONTROLLER_FAN_MAX_DUTY_PCT 100U
#define REAR_CONTROLLER_FAN_RAMP_MIN_C 45
#define REAR_CONTROLLER_FAN_RAMP_MAX_C 55

/**
 * @brief   Initializes the fan sub-system
 * @param   storage Pointer to the rear controller storage
 * @return  STATUS_CODE_OK if initialized succesfully
 *          STATUS_CODE_INVALID_ARGS if invalid parameter is passed in
 */
StatusCode fans_init(RearControllerStorage *storage);

/**
 * @brief   Ramps both PWM fans from idle to full over the temp band, and drives fan_pwr_1 on BPS fault
 * @return  STATUS_CODE_OK if executed succesfully
 */
StatusCode fans_run(void);

/** @} */
