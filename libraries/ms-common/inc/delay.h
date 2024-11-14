#pragma once

/************************************************************************************************
 * delay.h
 *
 * Header file for the delay library
 *
 * Created: 2024-10-30
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */

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
#define delay_s(time) delay_ms((time)*1000)
