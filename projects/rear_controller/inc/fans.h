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
 * @defgroup fans
 * @brief    fans Firmware
 * @{
 */

#define FAN_TEMP_THRESHOLD_C 40
#define FAN_TEMP_INVALID -100

/**
 * @brief   Initializes the fan sub-system
 * @return  STATUS_CODE_OK if initialized succesfully
 */
StatusCode fans_init(RearControllerStorage *storage);

/**
 * @brief   Checks the max board and cell temperatures in storage, compares them against the fan threshold, and turns on or off accordingly
 * @return  STATUS_CODE_OK if executed succesfully
 */
StatusCode fans_run(RearControllerStorage *storage);

/** @} */
