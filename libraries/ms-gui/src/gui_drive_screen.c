/************************************************************************************************
 * @file   gui_drive_screen.c
 *
 * @brief  High-level drive screen composition implementation
 *
 * @date   2026-04-22
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "clut.h"
#include "gui_drive_screen.h"
#include "gui_widgets.h"
#include "lvgl_screens.h"
#include "display_defs.h"

#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
#include "lvgl_widgets.h"

static SpeedometerWidget s_speedometer;
static BarWidget s_throttle_bar;
static BarWidget s_brake_bar;
static LabelWidget s_cc_label;

static bool s_drive_widgets_initialized;

static StatusCode s_create_speedometer(GuiScreen *screen) {
  const SpeedometerWidgetConfig speedometer_config = {
    .size = { .width = 220, .height = 220 },
    .position = {
      .type = WIDGET_POSITION_ALIGN,
      .value.align = { .align = WIDGET_ALIGN_CENTER, .x_offset = 0, .y_offset = 15 },
    },
    .total_tick_count = 41,
    .major_tick_every = 5,
    .angle_range = 270,
    .rotation = 135,
    .needle_length = -15,
  };

  return lvgl_widgets_create_speedometer(&s_speedometer, &speedometer_config, screen);
}

static StatusCode s_create_throttle_bar(GuiScreen *screen) {
  const BarWidgetConfig throttle_bar_config = {
    .size = { .width = 40, .height = 100 },
    .position = {
      .type = WIDGET_POSITION_ALIGN,
      .value.align = { .align = WIDGET_ALIGN_IN_TOP_LEFT, .x_offset = 15, .y_offset = 35 },
    },
    .label_text = "Throttle",
    .label_alignment = WIDGET_ALIGN_OUT_TOP_MID,
    .orientation = WIDGET_ORIENTATION_VERTICAL,
    .indicator_color_id = GUI_COLOR_THROTTLE_FILL,
  };

  return lvgl_widgets_create_bar(&s_throttle_bar, &throttle_bar_config, screen);
}

static StatusCode s_create_brake_bar(GuiScreen *screen) {
  const BarWidgetConfig brake_bar_config = {
    .size = { .width = 40, .height = 100 },
    .position = {
      .type = WIDGET_POSITION_ALIGN,
      .value.align = { .align = WIDGET_ALIGN_IN_BOTTOM_LEFT, .x_offset = 15, .y_offset = -35 },
    },
    .label_text = "Brake",
    .label_alignment = WIDGET_ALIGN_OUT_BOTTOM_MID,
    .orientation = WIDGET_ORIENTATION_VERTICAL,
    .indicator_color_id = GUI_COLOR_BRAKE_FILL,
  };

  return lvgl_widgets_create_bar(&s_brake_bar, &brake_bar_config, screen);
}

static StatusCode s_create_cc_label(GuiScreen *screen) {
  const LabelWidgetConfig cruise_control_label_config = {
    .size = { .width = 100, .height = 20 },
    .position = { .type = WIDGET_POSITION_ALIGN, .value.align = { .align = WIDGET_ALIGN_IN_BOTTOM_MID, .x_offset = 0, .y_offset = -30 } },
    .label_text = "0 km/h",
    .alignment = WIDGET_TEXT_ALIGN_CENTER,
    .text_color_id = GUI_COLOR_TEXT_PRIMARY,
    .font = GUI_SMALL_TEXT,
    .background_enabled = false,
    .background_color_id = 0,
    .border_enabled = false,
    .border_color_id = GUI_COLOR_LABEL_BORDER,
    .border_width = 0,
  };

  return lvgl_widgets_create_label(&s_cc_label, &cruise_control_label_config, screen);
}

static StatusCode s_drive_widgets_init_screen(GuiScreen *screen) {
  if (screen == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_drive_widgets_initialized) {
    return STATUS_CODE_ALREADY_INITIALIZED;
  }

  status_ok_or_return(s_create_speedometer(screen));
  status_ok_or_return(s_create_throttle_bar(screen));
  status_ok_or_return(s_create_brake_bar(screen));
  status_ok_or_return(s_create_cc_label(screen));

  s_drive_widgets_initialized = true;
  return STATUS_CODE_OK;
}

static void s_drive_widgets_deinit(void) {
  s_speedometer = (SpeedometerWidget){ 0 };
  s_throttle_bar = (BarWidget){ 0 };
  s_brake_bar = (BarWidget){ 0 };
  s_cc_label = (LabelWidget){ 0 };
  s_drive_widgets_initialized = false;
}

StatusCode gui_drive_screen_init(GuiScreen *screen) {
  if (screen == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  status_ok_or_return(lvgl_set_background_color(screen, GUI_COLOR_SCREEN_BACKGROUND));
  status_ok_or_return(gui_widgets_init_screen(screen));

  StatusCode status = s_drive_widgets_init_screen(screen);
  if (status != STATUS_CODE_OK) {
    s_drive_widgets_deinit();
    gui_widgets_deinit();
    return status;
  }

  return STATUS_CODE_OK;
}

void gui_drive_screen_deinit(void) {
  s_drive_widgets_deinit();
  gui_widgets_deinit();
}

StatusCode gui_drive_screen_widget_set_speed(int16_t speed_kmh) {
  if (!s_drive_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  return lvgl_widgets_set_speed(&s_speedometer, speed_kmh);
}

StatusCode gui_drive_screen_widget_set_throttle_bar(uint8_t percent) {
  if (!s_drive_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  return lvgl_widgets_set_bar_value(&s_throttle_bar, percent);
}

StatusCode gui_drive_screen_widget_set_brake_bar(uint8_t percent) {
  if (!s_drive_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  return lvgl_widgets_set_bar_value(&s_brake_bar, percent);
}

StatusCode gui_drive_screen_widget_set_brake_bar_color(GuiColorId color_id) {
  if (!s_drive_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  return lvgl_widgets_set_bar_color(&s_brake_bar, color_id);
}

StatusCode gui_drive_screen_widget_set_cc_speed(uint16_t cruise_control_speed_kmh, bool is_cc_enabled) {
  if (!s_drive_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  char text_buffer[LABEL_MAX_CHARS];
  if (is_cc_enabled) {
    snprintf(text_buffer, sizeof(text_buffer), "%u km/h", cruise_control_speed_kmh);
  } else {
    snprintf(text_buffer, sizeof(text_buffer), "cc off");
  }

  return lvgl_widgets_set_label_text(&s_cc_label, text_buffer);
}

StatusCode gui_widgets_set_brake_bar_color(GuiColorId color_id) {
  return lvgl_widgets_set_bar_color(&s_brake_bar, color_id);
}
#else

StatusCode gui_drive_screen_init(GuiScreen *screen) {
  (void)screen;
  return STATUS_CODE_OK;
}

void gui_drive_screen_deinit(void) {}

StatusCode gui_drive_screen_widget_set_speed(int16_t speed_kmh) {
  (void)speed_kmh;
  return STATUS_CODE_OK;
}

StatusCode gui_drive_screen_widget_set_throttle_bar(uint8_t percent) {
  (void)percent;
  return STATUS_CODE_OK;
}

StatusCode gui_drive_screen_widget_set_brake_bar(uint8_t percent) {
  (void)percent;
  return STATUS_CODE_OK;
}

StatusCode gui_drive_screen_widget_set_brake_bar_color(GuiColorId color_id) {
  (void)color_id;
  return STATUS_CODE_OK;
}

StatusCode gui_drive_screen_widget_set_cc_speed(uint16_t cruise_control_speed_kmh, bool is_cc_enabled) {
  (void)cruise_control_speed_kmh;
  (void)is_cc_enabled;
  return STATUS_CODE_OK;
}

StatusCode gui_widgets_set_brake_bar_color(GuiColorId color_id) {
  (void)color_id;
  return STATUS_CODE_OK;
}
#endif
