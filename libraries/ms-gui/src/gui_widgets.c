/************************************************************************************************
 * @file   gui_widgets.c
 *
 * @brief  High-level LVGL widget abstractions implementation
 *
 * @date   2026-03-10
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "lvgl.h"

/* Intra-component Headers */
#include "clut.h"
#include "gui_widgets.h"
#include "lvgl_screens.h"
#include "lvgl_widgets.h"

static SpeedometerWidget s_speedometer;
static BarWidget s_throttle_bar;
static BarWidget s_brake_bar;
static bool s_widgets_initialized;

static StatusCode s_create_speedometer(lv_obj_t *screen) {
  const SpeedometerWidgetConfig speedometer_config = {
    .size = { .width = 220, .height = 220 },
    .position = {
      .type = WIDGET_POSITION_ALIGN,
      .value.align = { .align = WIDGET_ALIGN_CENTER, .x_offset = 0, .y_offset = 0 },
    },
    .total_tick_count = 41,
    .major_tick_every = 5,
    .angle_range = 270,
    .rotation = 135,
    .needle_length = -15,
  };

  return lvgl_widgets_create_speedometer(&s_speedometer, &speedometer_config, screen);
}

static StatusCode s_create_throttle_bar(lv_obj_t *screen) {
  const BarWidgetConfig s_throttle_bar_config = {
    .size = { .width = 40, .height = 100 },
    .position = {
      .type = WIDGET_POSITION_ALIGN,
      .value.align = { .align = WIDGET_ALIGN_IN_TOP_LEFT, .x_offset = 15, .y_offset = 35 },
    },
    .label_text = "Throttle",
    .label_text_alignment = WIDGET_ALIGN_OUT_TOP_MID,
    .orientation = WIDGET_ORIENTATION_VERTICAL,
    .indicator_color_id = GUI_COLOR_THROTTLE_FILL,
  };

  return lvgl_widgets_create_bar(&s_throttle_bar, &s_throttle_bar_config, screen);
}

static StatusCode s_create_brake_bar(lv_obj_t *screen) {
  const BarWidgetConfig s_brake_bar_config = {
    .size = { .width = 40, .height = 100 },
    .position = { .type = WIDGET_POSITION_ALIGN, .value.align = { .align = WIDGET_ALIGN_IN_BOTTOM_LEFT, .x_offset = 15, .y_offset = -35 } },
    .label_text = "Brake",
    .label_text_alignment = WIDGET_ALIGN_OUT_BOTTOM_MID,
    .orientation = WIDGET_ORIENTATION_VERTICAL,
    .indicator_color_id = GUI_COLOR_BRAKE_FILL,
  };

  return lvgl_widgets_create_bar(&s_brake_bar, &s_brake_bar_config, screen);
}

StatusCode gui_widgets_init(void) {
  GuiScreen *screen = lvgl_get_active_screen();

  if (screen == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  status_ok_or_return(lvgl_set_background_color(screen, GUI_COLOR_SCREEN_BACKGROUND));

  /* Create widgets */
  status_ok_or_return(s_create_speedometer(screen));
  status_ok_or_return(s_create_throttle_bar(screen));
  status_ok_or_return(s_create_brake_bar(screen));
  s_widgets_initialized = true;

  return STATUS_CODE_OK;
}

StatusCode gui_widgets_set_speed(int16_t speed_kmh) {
  if (!s_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  return lvgl_widgets_set_speed(&s_speedometer, speed_kmh);
}

StatusCode gui_widgets_set_throttle_bar(uint8_t percent) {
  if (!s_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  return lvgl_widgets_set_bar_value(&s_throttle_bar, percent);
}

StatusCode gui_widgets_set_brake_bar(uint8_t percent) {
  if (!s_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  return lvgl_widgets_set_bar_value(&s_brake_bar, percent);
}

StatusCode gui_widgets_set_soc_bar(uint8_t soc_percent) {
  (void)soc_percent;
  return STATUS_CODE_UNIMPLEMENTED;
}
