#pragma once

/************************************************************************************************
 * @file   xb_transmit.h
 *
 * @brief  XBee transmit header file
 *
 * @date   2026-02-11
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */
#include "gpio.h"

/* Intra-component Headers */
#include "status.h"

/**
 * @defgroup telemetry
 * @brief    telemetry Firmware
 * @{
 */

/**
 * @brief   attempts to pop a can message, if successful sends it to XB
 */
StatusCode xb_transmit_run();

/**
 * @brief   Initializes the xb_transmit component
 * @return  STATUS_CODE_OK if initialization is successful
 *          STATUS_CODE_INVALID_ARGS if storage is null
 */
StatusCode xb_transmit_init(TelemetryStorage *storage);

/** @} */
