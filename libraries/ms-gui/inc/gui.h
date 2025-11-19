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
#include "framebuffer.h"
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
 * @brief   GUI initialization settings
 */
typedef struct {
  LtdcSettings ltdc;       /**< LTDC driver configuration */
  Framebuffer framebuffer; /**< GUI framebuffer */
} GuiSettings;

/**
 * @brief   Initialize GUI subsystem (framebuffer, CLUT, LTDC)
 * @param   settings Pointer to GUI configuration
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_init(GuiSettings *settings);

/**
 * @brief   Draw a single pixel at (x, y)
 * @param   x X coordinate
 * @param   y Y coordinate
 * @param   color_index CLUT color index
 */
void gui_draw_pixel(uint16_t x, uint16_t y, ColorIndex color_index);

/**
 * @brief   Draw a filled rectangle
 * @param   x X coordinate of the top left corner of the rectangle
 * @param   y Y coordinate of the top left corner of the rectangle
 * @param   width Width in pixels
 * @param   height Height in pixels
 * @param   color_index CLUT color index
 */
void gui_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, ColorIndex color_index);

/**
 * @brief   Draw a line
 * @param   x0 X coordinate start location
 * @param   y0 Y coordinate start location
 * @param   x1 X coordinate end location
 * @param   y1 Y coordinate end location
 * @param   color_index CLUT color index
 */
void gui_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, ColorIndex color_index);

/**
 * @brief   Update display with current framebuffer
 */
StatusCode gui_render(void);



void gui_progress_bar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t percentage, ColorIndex color_index_bg, ColorIndex color_index_fg);

void gui_display_char(uint16_t x, uint16_t y, char c, ColorIndex color_index);

void gui_display_text(uint16_t x, uint16_t y, const char *text, ColorIndex color_index);

/** @} */
