#pragma once

/************************************************************************************************
 * @file    state_of_charge.h
 *
 * @brief   State of charge header file
 *
 * @date    2025-10-12
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "rear_controller.h"
#include "rear_controller_hw_defs.h"

/**
 * @defgroup Rear_Controller
 * @brief    Rear Controller Board Firmware
 * @{
 */

/**
 * @brief   Initialize the state of charge EKF
 * @param   storage Pointer to the rear controller storage
 * @return  STATUS_CODE_OK when initialized successfully
 *          STATUS_CODE_INVALID_ARGS when invalid parameters are passed
 */
StatusCode state_of_charge_init(RearControllerStorage *storage);

/**
 * @brief   Run state of charge calculation
 * @return  STATUS_CODE_OK when calculated successfully
 */
StatusCode state_of_charge_run();

/** @} */
