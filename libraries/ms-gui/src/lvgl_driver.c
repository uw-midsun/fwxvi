/************************************************************************************************
 * @file   lvgl_driver.c
 *
 * @brief  LVGL display driver adapter implementation
 *
 * @date   2026-03-10
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#ifdef STM32L4P5xx
#include "FreeRTOS.h"
#include "lvgl.h"
#include "task.h"
#endif

/* Intra-component Headers */
#include "ltdc.h"
#include "lvgl_driver.h"

#ifdef STM32L4P5xx
static LtdcSettings *s_ltdc_settings;

/* LVGL draw buffer (We use partial... Reccomended is 1/10 display size) */
#define LV_DRAW_BUF_LINES 20
#define NUMBER_OF_BYTES_PER_PIXEL 2 /* Since RGB565 = 2 bytes per pixel*/
static uint8_t s_draw_buf[DISPLAY_WIDTH * LV_DRAW_BUF_LINES * NUMBER_OF_BYTES_PER_PIXEL];

/**
 * @brief   LVGL display flush callback
 * @details Copies the rendered area from LVGL's draw buffer into the LTDC framebuffer.
 * @param   display   lvgl display object
 * @param   area      Area we want to update
 * @param   px_map    Pixels of the new area
 */
static void s_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map) {
  // TODO Kind of a slow approach could optimize with DMA2d
  uint16_t *framebuffer = (uint16_t *)s_ltdc_settings->framebuffer;
  uint16_t *src = (uint16_t *)px_map;
  uint16_t width = lv_area_get_width(area);

  for (int32_t y = area->y1; y <= area->y2; ++y) {
    memcpy(&framebuffer[y * s_ltdc_settings->width + area->x1], src, width * sizeof(uint16_t));
    src += width;
  }

  ltdc_draw();

  /* Signal to LVGL that flushing is complete */
  lv_display_flush_ready(display);
}

/**
 * @brief   LVGL tick provider
 * @return  Elapsed milliseconds since boot
 */
static uint32_t s_tick_get_cb(void) {
  return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

StatusCode lvgl_driver_init(LtdcSettings *settings) {
  if (settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_ltdc_settings = settings;

  lv_init();

  /* Set tick */
  lv_tick_set_cb(s_tick_get_cb);

  /* Create display and configure flush callback */
  lv_display_t *disp = lv_display_create(settings->width, settings->height);
  if (disp == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
  lv_display_set_flush_cb(disp, s_flush_cb);
  lv_display_set_buffers(disp, s_draw_buf, NULL, sizeof(s_draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

  return STATUS_CODE_OK;
}

StatusCode lvgl_driver_process(void) {
  lv_timer_handler();
  return STATUS_CODE_OK;
}
#else
StatusCode lvgl_driver_init(LtdcSettings *settings) {
  (void)settings;
  return STATUS_CODE_OK;
}

StatusCode lvgl_driver_process(void) {
  return STATUS_CODE_OK;
}
#endif
