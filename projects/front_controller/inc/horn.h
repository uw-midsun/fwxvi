#pragma once

/************************************************************************************************
 * @file   horn.h
 *
 * @brief  Header file for horn manager
 *
 * @date   2025-11-17
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "front_controller.h"

/**
 * @defgroup Front_Controller
 * @brief    Front Controller Board Firmware
 * @{
 */

/**
 * @brief   Reads the horn CAN status and sets the enable pin
 * @return  STATUS_CODE_OK if pin is set successfully
 *          STATUS_CODE_INTERNAL_ERROR if CAN message is not received
 */
StatusCode horn_run();

/**
 * @brief   Initializes the horn pin
 * @return  STATUS_CODE_OK if horn is initialized successfully
 */
StatusCode horn_init();

/** @} */
