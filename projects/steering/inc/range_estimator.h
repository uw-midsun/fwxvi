#pragma once

/************************************************************************************************
 * @file   range_estimator.h
 *
 * @brief  Header file for estimating driving range based on remaining voltage
 *
 * @date   2025-10-04
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "steering.h"

/* Intra-component Headers */

/**
 * @defgroup steering
 * @brief    steering Firmware
 * @{
 */

/**
 * @brief   Run the range estimator which estimates remaining kilometres based on cell voltage
 */
StatusCode range_estimator_run();

/**
 * @brief   Initialize the range estimator component
 * @param   storage storage struct for all steering data
 * @return  STATUS_CODE_OK if initialization is successful
 *          STATUS_CODE_INVALID_ARGS if storage is null
 */
StatusCode range_estimator_init(SteeringStorage *storage);

/** @} */
