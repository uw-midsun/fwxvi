/************************************************************************************************
 * @file   gui.c
 *
 * @brief  Source file for the GUI library
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/
/* Standard library Headers */
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */
#include "clut.h"
#include "gui.h"
#include "gui_widgets.h"
#include "ltdc.h"
#include "lvgl_driver.h"

/* Intra-component Headers */
#include "log.h"
#include "status.h"

/* Standard Library Headers */

StatusCode gui_init(LtdcSettings *settings) {
  if (!settings) {
    return STATUS_CODE_INVALID_ARGS;
  }

  StatusCode ret;

  ret = ltdc_init(settings);
  if (ret != STATUS_CODE_OK) {
    LOG_DEBUG("ltdc_init error: %d\n", ret);
    return ret;
  }

  ret = lvgl_driver_init(settings);
  if (ret != STATUS_CODE_OK) {
    LOG_DEBUG("lvgl_driver_init error: %d\n", ret);
    return ret;
  }

  ret = gui_widgets_init();
  if (ret != STATUS_CODE_OK) {
    LOG_DEBUG("gui_widgets_init error: %d\n", ret);
  }

  return STATUS_CODE_OK;
}

StatusCode gui_render() {
  return lvgl_driver_process();
}

StatusCode gui_draw_pixel(uint16_t x, uint16_t y, ColorIndex color_index) {
  if (color_index >= NUM_COLOR_INDICES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  return ltdc_set_pixel(x, y, color_index);
}

StatusCode gui_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, ColorIndex color_index) {
  if (color_index >= NUM_COLOR_INDICES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  for (uint16_t i = 0; i < width; ++i) {
    for (uint16_t j = 0; j < height; ++j) {
      status_ok_or_return(ltdc_set_pixel(x + i, y + j, color_index));
    }
  }
  return STATUS_CODE_OK;
}

StatusCode gui_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, ColorIndex color_index) {
  if (color_index >= NUM_COLOR_INDICES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;

  while (1) {
    status_ok_or_return(ltdc_set_pixel(x0, y0, color_index));

    if (x0 == x1 && y0 == y1) {
      break;
    }

    int e2 = 2 * err;
    if (e2 >= -dy) {
      err -= dy;
      x0 += sx;
    }
    if (e2 <= dx) {
      err += dx;
      y0 += sy;
    }
  }
  return STATUS_CODE_OK;
}
