#pragma once

/************************************************************************************************
 * @file   gui_fault_screen.h
 *
 * @brief  Full-screen BPS fault takeover screen
 *
 * @date   2026-07-17
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "lvgl_screens.h"

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

/**
 * @brief   Build the BPS fault screen: a full red background with the fault name and cell
 * @param   screen Root screen object to populate
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_fault_screen_init(GuiScreen *screen);

/**
 * @brief   Reset fault-screen widget handles after the active root is cleaned
 */
void gui_fault_screen_deinit(void);

/**
 * @brief   Refresh the fault detail line with the latest fault bitmask and cell
 * @param   fault_code Latched BPS fault bitmask
 * @param   cell_at_fault One-based cell index for cell-related faults (0 when not applicable)
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_fault_screen_widget_set(uint16_t fault_code, uint8_t cell_at_fault);

/** @} */
