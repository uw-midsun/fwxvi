#pragma once

/************************************************************************************************
 * @file   gui.h
 *
 * @brief  Header file for the GUI library
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */
#include "clut.h"
#include "ltdc.h"

/* Intra-component Headers */
#include "status.h"

/* Standard Library Headers */

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

/**
 * @brief   Initialize the GUI subsystem
 * @param   settings Pointer to LTDC/display configuration
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_init(LtdcSettings *settings);

/**
 * @brief   Run one GUI processing/render step
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_render();

/** @} */
