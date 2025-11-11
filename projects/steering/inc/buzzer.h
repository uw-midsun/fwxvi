#pragma once

/************************************************************************************************
 * @file    buzzer.h
 *
 * @brief   Header file for steering buzzer
 *
 * @date    2025-07-28
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "gpio.h"
#include "status.h"

/* Intra-component Headers */

/**
 * @defgroup steering
 * @brief    steering Firmware
 * @{
 */

/**
 * @brief   Initialize the buzzer
 * @return  STATUS_CODE_OK if initialized successfully
 */
StatusCode buzzer_init();

/**
 * @brief   Trigger the buzzer to beep. If a beep is already registered, it will extend the existing beep
 * @return  STATUS_CODE_OK if initialized successfully
 */
StatusCode buzzer_beep();

/** @} */
