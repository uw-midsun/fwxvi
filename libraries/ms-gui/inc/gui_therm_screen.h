#pragma once

/************************************************************************************************
 * @file   gui_therm_screen.h
 *
 * @brief  High-level thermistor monitor screen composition
 *
 * @date   2026-07-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "lvgl_screens.h"

/**
 * @defgroup gui_therm_screen
 * @brief    gui_therm_screen Firmware
 * @{
 */

/**
 * @brief   Create the thermistor monitor screen (title + thermistor table)
 * @param   screen Screen root to populate
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_therm_screen_init(GuiScreen *screen);

/**
 * @brief   Reset cached thermistor-screen widget handles after their LVGL parent is deleted
 */
void gui_therm_screen_deinit(void);

/**
 * @brief   Update a thermistor table cell
 * @param   therm_idx Zero-based thermistor index
 * @param   thermistor_mv Raw AFE thermistor voltage in mV (not temperature)
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_therm_screen_widget_set_thermistor(uint8_t therm_idx, uint16_t thermistor_mv);

/** @} */
