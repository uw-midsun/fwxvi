#pragma once

/************************************************************************************************
 * @file   delay.h
 *
 * @brief  Header file for the delay library
 *
 * @date   2024-10-30
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup RTOS_Helpers
 * @brief    RTOS helper libraries
 * @{
 */

/**
 * @brief   Blocking delay for some amount of time in milliseconds
 * @param   time_ms Amount of time to delay for
 */
void delay_ms(uint32_t time_ms);

/**
 * @brief   Non-blocking delay for some amount of time in milliseconds
 * @param   time_ms Amount of time to delay for
 */
void non_blocking_delay_ms(uint32_t time_ms);

/**
 * @brief   Blocking delay for some amount of time in seconds
 * @param   time_ms Amount of time to delay for
 */
#define delay_s(time) delay_ms((time) * 1000)

/** @} */
