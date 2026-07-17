#pragma once

/************************************************************************************************
 * @file   gui_widgets.h
 *
 * @brief  Common high-level LVGL widgets for the vehicle display
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
 * @brief   Initialize the common widget layer on the active LVGL screen
 * @details Must be called after lvgl_driver_init().
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_init(void);

/**
 * @brief   Initialize the widget layer on a specific screen root
 * @param   screen Screen root to populate with common widgets
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_init_screen(GuiScreen *screen);

/**
 * @brief   Reset cached common widget handles after their LVGL parent is deleted
 */
void gui_widgets_deinit(void);

/**
 * @brief   Update the vertical temperature bar
 * @param   percent Current motor temperature
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_set_temperature_bar(uint16_t percent);

/**
 * @brief   Update the vertical motor speed bar
 * @param   percent Current motor speed
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_set_speed_bar(uint16_t percent);

/**
 * @brief   Update the horizontal segmented soc bar
 * @param   soc_percent Current percentage (0-100)
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_set_soc_bar(uint8_t soc_percent);

/**
 * @brief   Update the text for the top label
 * @param   pack_voltage The battery pack voltage
 * @param   pack_current The battery pack current
 * @param   motor_bus_voltage The motor bus voltage
 * @param   motor_bus_current The motor bus current
 * @param   fault Active BPS fault bitmask
 * @param   cell_at_fault One-based cell index for cell-related faults, or 0 if not applicable
 * @param   ws22_flags Motor flags bitmask
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_set_top_label(float pack_voltage, float pack_current, float motor_bus_voltage, float motor_bus_current, uint16_t bps_fault, uint8_t cell_at_fault, uint16_t ws22_flags);

/**
 * @brief   Map a BPS fault bitmask to a human-readable name (highest-priority active fault)
 * @param   fault Active BPS fault bitmask
 * @param   is_cell_fault Out param set true when the primary fault is cell-associated, or NULL
 * @return  Static fault name string ("BPS FAULT" when none/NULL)
 */
const char *gui_widgets_bps_fault_text(uint16_t fault, bool *is_cell_fault);

/**
 * @brief   Show/refresh/hide the full-screen BPS fault popup based on the live fault bitmask
 * @details Draws a high-visibility overlay on the LVGL top layer so a BPS fault is warned on every
 *          screen (ASC 2026 8.7.B), and tears it down once the fault bitmask clears. Safe to call
 *          every render cycle; the overlay is created lazily on the first fault.
 * @param   bps_fault Active BPS fault bitmask (0 = no fault)
 * @param   cell_at_fault One-based cell index for cell-related faults, or 0 if not applicable
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_bps_popup_update(uint16_t bps_fault, uint8_t cell_at_fault);

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
 * @brief   Update the shared aux battery voltage + energy-used label
 * @param   aux_mv Auxiliary battery voltage in mV (signed)
 * @param   energy_wh Net energy drawn from the pack in Wh
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_set_aux_energy_label(int16_t aux_mv, float energy_wh);

/** @} */
