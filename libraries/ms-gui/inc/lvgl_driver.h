#pragma once

/************************************************************************************************
 * @file   lv_driver.h
 *
 * @brief  LVGL display driver adapter for the LTDC/framebuffer pipeline
 *
 * @date   2026-03-10
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "status.h"
/* Intra-component Headers */
#include "ltdc.h"
#include "display_defs.h"

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

/**
 * @brief   Initialize the LVGL display driver
 * @details Creates an LVGL display, registers the flush callback,
 *          and configures the tick provider using FreeRTOS.
 *          Must be called after ltdc_init().
 * @param   settings Pointer to the LTDC settings (used for width/height/framebuffer)
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lv_driver_init(LtdcSettings *settings);

/**
 * @brief   Run one LVGL processing step
 * @details Call this periodically from the one task that owns all LVGL access.
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lv_driver_process(void);

/** @} */
