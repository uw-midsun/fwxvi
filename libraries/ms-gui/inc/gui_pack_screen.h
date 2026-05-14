#pragma once

/************************************************************************************************
 * @file   gui_pack_screen.h
 *
 * @brief  High-level pack voltage screen composition
 *
 * @date   2026-04-17
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "lvgl_screens.h"

/**
 * @defgroup gui_pack_screen
 * @brief    gui_pack_screen Firmware
 * @{
 */

/**
 * @brief   Create the pack voltage screen from common widgets and the pack table
 * @param   screen Screen root to populate
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_pack_screen_init(GuiScreen *screen);

/**
 * @brief   Reset cached pack-voltage widget handles after their LVGL parent is deleted
 */
void gui_pack_screen_deinit(void);

/**
 * @brief   Update a pack-voltage table cell
 * @param   cell_idx Zero-based cell index
 * @param   cell_voltage Cell voltage in the same fixed-point mV units used by the BPS CAN signal
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_pack_screen_widget_set_pack_voltage(uint8_t cell_idx, uint16_t cell_voltage);

/**
 * @brief   Update the speedometer label
 * @param   speed_kmh Current speed in km/h
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_pack_screen_widget_set_speed_label(int16_t speed_kmh);

/**
 * @brief   Update the cruise control set speed
 * @param   cruise_control_speed_kmh The cruise control set speed in km/h
 * @param   is_cc_enabled Whether cruise control is enabled
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_pack_screen_widget_set_cc_speed(uint16_t cruise_control_speed_kmh, bool is_cc_enabled);

/** @} */
