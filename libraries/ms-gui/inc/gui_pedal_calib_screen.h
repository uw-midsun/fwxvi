#pragma once

/************************************************************************************************
 * @file   gui_pedal_calib_screen.h
 *
 * @brief  High-level pedal calibration screen
 *
 * @date   2026-04-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "clut.h"
#include "lvgl_screens.h"

/**
 * @defgroup gui_pedal_calib_screen
 * @brief    gui_pedal_calib_screen Firmware
 * @{
 */

/**
 * @brief   Create the pedal calibration screen
 * @param   screen Screen root to populate
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_pedal_calib_screen_init(GuiScreen *screen);

/**
 * @brief   Reset cached pedal calib widget handles after their LVGL parent is deleted
 */
void gui_pedal_calib_screen_deinit(void);

/**
 * @brief   Update the large display text with calibration status
 * @param   text Text to display (e.g., "LIFT Accel Pedal")
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_pedal_calib_widget_big_text(const char *text);

/**
 * @brief   Update the subtitle text shown below the main status label
 * @param   text Text to display (pass "" to clear)
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_pedal_calib_widget_subtitle_text(const char *text);

/**
 * @brief   Toggle the BPS-fault takeover styling on the pedal calibration screen
 * @details When active, the screen shows the red BPS fault background with a "BPS FAULT" title and
 *          the decoded fault name (and cell, when applicable), matching the old dedicated fault
 *          screen. When inactive, the normal pedal calibration appearance is restored. This lets a
 *          BPS fault reuse the already-allocated pedal calib screen instead of a separate one.
 * @param   fault_active TRUE to force the fault appearance, FALSE to restore normal calibration UI
 * @param   fault_code BPS fault bitfield used to decode the fault name
 * @param   cell_at_fault Cell number associated with the fault, or 0 when not cell-specific
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_pedal_calib_screen_set_fault(bool fault_active, uint16_t fault_code, uint8_t cell_at_fault);

/**
 * @brief   Query whether the pedal calibration screen is currently in BPS-fault takeover mode
 * @return  TRUE while the fault appearance is forced, FALSE otherwise
 */
bool gui_pedal_calib_screen_is_fault_active(void);

/** @} */
