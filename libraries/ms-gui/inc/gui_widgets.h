#pragma once

/************************************************************************************************
 * @file   gui_widgets.h
 *
 * @brief  High-level LVGL widget abstractions for the vehicle display
 *
 * @date   2026-03-10
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
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
 * @brief   Initialize the widget layer and create the main screen
 * @details Sets up common LVGL styles, creates the speedometer (round scale)
 *          and vertical percentage bar. Must be called after lvgl_driver_init().
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_init(void);

/**
 * @brief   Initialize the widget layer on a specific screen root
 * @param   screen Screen root to populate with the drive widgets
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_init_screen(GuiScreen *screen);

/**
 * @brief   Update the speedometer needle
 * @param   speed_kmh Current speed in km/h
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_set_speed(int16_t speed_kmh);

/**
 * @brief   Update the vertical throttle percentage bar
 * @param   percent Current percentage (0-100)
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_set_throttle_bar(uint8_t percent);

/**
 * @brief   Update the vertical brake percentage bar
 * @param   percent Current percentage (0-100)
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_set_brake_bar(uint8_t percent);

/**
 * @brief   Update the horizontal segmented soc bar
 * @param   soc_percent Current percentage (0-100)
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_set_soc_bar(uint8_t soc_percent);

/**
 * @brief   Update the text for the top label
 * @param   pack_voltage The battery pack voltage
 * @param   motor_bus_voltage The motor bus voltage
 * @param   fault Active BPS fault bitmask
 * @param   cell_at_fault One-based cell index for cell-related faults, or 0 if not applicable
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_set_top_label(uint16_t pack_voltage, uint16_t motor_bus_voltage, uint16_t fault, uint8_t cell_at_fault);

/**
 * @brief   Update the text for the cell stats label
 * @param   min_cell_voltage_mv The minimum cell voltage in mV
 * @param   max_cell_voltage_mv The maximum cell voltage in mV
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_set_cell_stats_label(uint16_t min_cell_voltage_mv, uint16_t max_cell_voltage_mv);

/**
 * @brief   Update the text for the temperature stats label
 * @param   motor_temp_c The motor temperature in C
 * @param   max_cell_temp_c The maximum cell temperature in C
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_set_temps_stats_label(int16_t motor_temp_c, uint16_t max_cell_temp_c);

/**
 * @brief   Update the cruise control set speed
 * @param   cruise_control_speed_kmh The cruise control set speed in km/h
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_set_cc_speed(uint16_t cruise_control_speed_kmh, bool is_cc_enabled);
/** @} */
