#pragma once

/************************************************************************************************
 * @file   curretn_sense.h
 *
 * @brief Current sensing for Rear_Controller
 *
 * @date   2025-09-17
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "rear_controller.h"

/**
 * @defgroup Rear_Controller
 * @brief    Rear Controller Board Firmware
 * @{
 */

StatusCode current_sense_run();

StatusCode current_sense_init(RearControllerStorage *rear_controller_storage);

/** @} */
