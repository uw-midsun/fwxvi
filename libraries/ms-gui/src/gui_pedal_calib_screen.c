/************************************************************************************************
 * @file   gui_pedal_calib_screen.c
 *
 * @brief  High-level pedal calibration screen implementation
 *
 * @date   2026-04-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "clut.h"
#include "display_defs.h"
#include "gui_pedal_calib_screen.h"
#include "gui_widgets.h"

#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
#include "lvgl_screens.h"
#include "lvgl_widgets.h"

static LabelWidget s_status_label;
static bool s_pedal_calib_widgets_initialized;

static StatusCode s_create_status_label(GuiScreen *screen) {
  const LabelWidgetConfig status_label_config = {
    .size = { .width = 320, .height = 140 },
    .position = {
      .type = WIDGET_POSITION_ALIGN,
      .value.align = { .align = WIDGET_ALIGN_CENTER, .x_offset = 0, .y_offset = 0 },
    },
    .label_text = "Pedal Calibration",
    .alignment = WIDGET_TEXT_ALIGN_CENTER,
    .text_color_id = GUI_COLOR_TEXT_PRIMARY,
    .font = GUI_BIG_TEXT,
    .background_enabled = false,
    .background_color_id = 0,
    .border_enabled = false,
    .border_color_id = GUI_COLOR_LABEL_BORDER,
    .border_width = 0,
  };

  return lvgl_widgets_create_label(&s_status_label, &status_label_config, screen);
}

StatusCode gui_pedal_calib_screen_init(GuiScreen *screen) {
  if (screen == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_pedal_calib_widgets_initialized) {
    return STATUS_CODE_ALREADY_INITIALIZED;
  }

  status_ok_or_return(lvgl_set_background_color(screen, GUI_COLOR_SCREEN_BACKGROUND));

  status_ok_or_return(s_create_status_label(screen));
  
  s_pedal_calib_widgets_initialized = true;
  return STATUS_CODE_OK;
}

void gui_pedal_calib_screen_deinit(void) {
  s_status_label = (LabelWidget){ 0 };
  s_pedal_calib_widgets_initialized = false;
}

StatusCode gui_pedal_calib_widget_big_text(const char *text) {
  if (!s_pedal_calib_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (text == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  return lvgl_widgets_set_label_text(&s_status_label, text);
}

#else

StatusCode gui_pedal_calib_screen_init(GuiScreen *screen) {
  (void)screen;
  return STATUS_CODE_OK;
}

void gui_pedal_calib_screen_deinit(void) {}

StatusCode gui_pedal_calib_widget_big_text(const char *text) {
  (void)text;
  return STATUS_CODE_OK;
}

#endif
