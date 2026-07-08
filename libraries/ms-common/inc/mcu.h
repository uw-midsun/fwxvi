#pragma once

/************************************************************************************************
 * @file   mcu.h
 *
 * @brief  Header file for MCU intialization
 *
 * @date   2024-10-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "status.h"

/**
 * @defgroup MCU
 * @brief    MCU library
 * @{
 */

/* Timeout for HSE/LSE pins to stabilize. Should not take more than 5 seconds */
#define HSE_LSE_TIMEOUT_MS 5000U

#define USE_INTERNAL_OSCILLATOR 1U

/**
 * @brief   Configure the system clock (PLL to 80 MHz, flash latency, bus dividers)
 * @details Standalone from mcu_init so a minimal client (for example the bootloader) can bring the
 *          clock up after HAL_Init without pulling in GPIO and printf retargeting
 */
StatusCode mcu_clock_init(void);

/**
 * @brief   Initialize MCU system clock and HAL.
 * @details Call this function once at the start of your application.
 */
StatusCode mcu_init(void);

/** @} */
