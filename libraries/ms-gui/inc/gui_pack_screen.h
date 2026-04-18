#pragma once

/************************************************************************************************
 * @file   gui_pack_screen.h
 *
 * @brief  High-level screen registry and navigation for cell voltages
 *
 * @date   2026-04-17
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "lvgl_screens.h"

StatusCode gui_pack_screen_init(GuiScreen *screen);

StatusCode gui_pack_screen_set_voltages(const uint16_t *cell_voltages);

StatusCode gui_widgets_set_pack_voltage(uint8_t cell_idx, uint16_t cell_voltage);
