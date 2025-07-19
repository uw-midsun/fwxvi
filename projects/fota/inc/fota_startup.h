#pragma once

/************************************************************************************************
 * @file   fota_startup.h
 *
 * @brief  Header file for FOTA startup
 *
 * @date   2025-03-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_error.h"

/**
 * @defgroup FOTA
 * @brief    Firmware Over the Air (FOTA) Update system
 * @{
 */

/* Timeout for HSE/LSE pins to stabilize. Should not take more than 5 seconds */
#define HSE_LSE_TIMEOUT_MS 5000U

#define USE_INTERNAL_OSCILLATOR 1U

/**
 * @brief   Initialize MCU system clock and HAL
 * @details Call this function once at the start of your application
 */
FotaError fota_startup(void);

/** @} */
