#pragma once

/************************************************************************************************
 * @file   thread_helpers.h
 *
 * @brief  Header file defining the thread helper functions
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup ThreadHelpers
 * @brief    Thread helper/management library
 * @{
 */

/**
 * @brief   Sleep the current Linux pthread with the specified seconds
 * @param   seconds Seconds to block the current thread
 */
void thread_sleep_s(unsigned int seconds);

/**
 * @brief   Sleep the current Linux pthread with the specified milliseconds
 * @param   milliseconds Milliseconds to block the current thread
 */
void thread_sleep_ms(unsigned int milliseconds);

/** @} */
