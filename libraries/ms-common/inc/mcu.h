#pragma once

/************************************************************************************************
 * @file   mcu.h
 *
 * @brief  Header file for MCU intialization
 *
 * @date   2024-10-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */

/* Intra-component Headers*/
#include "status.h"

/* Timeout for HSE/LSE pins to stabilize. Should not take more than 5 seconds */
#define HSE_LSE_TIMEOUT_MS 5000

/**
 * @brief   Initialize MCU system clock and HAL.
 * @details Call this function once at the start of your application.
 */
StatusCode mcu_init(void);
