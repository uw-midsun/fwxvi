#pragma once

/************************************************************************************************
 * @file   motor_can.h
 *
 * @brief  Header file for motor can
 *
 * @date   2025-11-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "can.h"
#include "status.h"

/* Intra-component Headers */
#include "rear_controller.h"
#include "ws22_motor_can.h"

/**
 * @defgroup Rear Controller
 * @brief    Rear Controller Board Firmware
 * @{
 */

StatusCode motor_can_process_rx(CanMessage *msg);
