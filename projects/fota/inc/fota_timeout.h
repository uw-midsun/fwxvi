#pragma once

/************************************************************************************************
 * @file    fota_timeout.h
 *
 * @brief   FOTA Timeout
 *
 * @date    2025-06-14
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup FOTA
 * @brief    Firmware Over the Air (FOTA) Update system
 * @{
 */

/* @brief   FOTA timeout in milliseconds */
#define FOTA_TIMEOUT_MS 15000U

/**
 * @brief   Checks if FOTA has timed out, meaning no requests have been received
 * @return  TRUE if a timeout event has occurred
 *          FALSE if no timeout has occurred
 */
bool fota_is_timed_out(void);

/** @} */
