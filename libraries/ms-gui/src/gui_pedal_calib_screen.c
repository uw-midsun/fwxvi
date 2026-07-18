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
static LabelWidget s_subtitle_label;
static bool s_pedal_calib_widgets_initialized;
static GuiScreen *s_screen;
static bool s_fault_active;

static lv_color_t s_gui_palette_color(GuiColorId color_id) {
  ClutEntry color = clut_get_gui_color(color_id);
  return lv_color_make(clut_entry_red(color), clut_entry_green(color), clut_entry_blue(color));
}

static StatusCode s_create_status_label(GuiScreen *screen) {
  const LabelWidgetConfig status_label_config = {
    .size = { .width = 320, .height = 140 },
    .position = {
      .type = WIDGET_POSITION_ALIGN,
      .value.align = { .align = WIDGET_ALIGN_CENTER, .x_offset = 0, .y_offset = -20 },
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

static StatusCode s_create_subtitle_label(GuiScreen *screen) {
  const LabelWidgetConfig subtitle_label_config = {
    .size = { .width = 320, .height = 0 },
    .position = {
      .type = WIDGET_POSITION_ALIGN,
      .value.align = { .align = WIDGET_ALIGN_CENTER, .x_offset = 0, .y_offset = 40 },
    },
    .label_text = "press to continue",
    .alignment = WIDGET_TEXT_ALIGN_CENTER,
    .text_color_id = GUI_COLOR_TEXT_PRIMARY,
    .font = GUI_SMALL_TEXT,
    .background_enabled = false,
    .background_color_id = 0,
    .border_enabled = false,
    .border_color_id = GUI_COLOR_LABEL_BORDER,
    .border_width = 0,
  };

  return lvgl_widgets_create_label(&s_subtitle_label, &subtitle_label_config, screen);
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
  status_ok_or_return(s_create_subtitle_label(screen));

  s_screen = screen;
  s_fault_active = false;
  s_pedal_calib_widgets_initialized = true;
  return STATUS_CODE_OK;
}

void gui_pedal_calib_screen_deinit(void) {
  s_status_label = (LabelWidget){ 0 };
  s_subtitle_label = (LabelWidget){ 0 };
  s_screen = NULL;
  s_fault_active = false;
  s_pedal_calib_widgets_initialized = false;
}

StatusCode gui_pedal_calib_screen_set_fault(bool fault_active, uint16_t fault_code, uint8_t cell_at_fault) {
  if (!s_pedal_calib_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (fault_active) {
    lv_color_t fg_color = s_gui_palette_color(GUI_COLOR_BPS_FAULT_TEXT);

    status_ok_or_return(lvgl_set_background_color(s_screen, GUI_COLOR_BPS_FAULT_BACKGROUND));

    lv_obj_set_style_text_color(s_status_label.label, fg_color, 0);
    lv_obj_set_style_text_color(s_subtitle_label.label, fg_color, 0);
    lv_obj_set_style_text_font(s_subtitle_label.label, GUI_MEDIUM_TEXT, 0);
    status_ok_or_return(lvgl_widgets_set_label_text(&s_status_label, "BPS FAULT"));

    bool is_cell_fault = false;
    const char *fault_text = gui_widgets_bps_fault_text(fault_code, &is_cell_fault);
    char detail_buffer[LABEL_MAX_CHARS];
    if (is_cell_fault && cell_at_fault != 0U) {
      snprintf(detail_buffer, sizeof(detail_buffer), "%s\nCell %u", fault_text, cell_at_fault);
    } else {
      snprintf(detail_buffer, sizeof(detail_buffer), "%s", fault_text);
    }
    status_ok_or_return(lvgl_widgets_set_label_text(&s_subtitle_label, detail_buffer));
  } else {
    lv_color_t fg_color = s_gui_palette_color(GUI_COLOR_TEXT_PRIMARY);

    status_ok_or_return(lvgl_set_background_color(s_screen, GUI_COLOR_SCREEN_BACKGROUND));

    lv_obj_set_style_text_color(s_status_label.label, fg_color, 0);
    lv_obj_set_style_text_color(s_subtitle_label.label, fg_color, 0);
    lv_obj_set_style_text_font(s_subtitle_label.label, GUI_SMALL_TEXT, 0);
    status_ok_or_return(lvgl_widgets_set_label_text(&s_status_label, "Pedal Calibration"));
    status_ok_or_return(lvgl_widgets_set_label_text(&s_subtitle_label, "press to continue"));
  }

  s_fault_active = fault_active;
  return STATUS_CODE_OK;
}

bool gui_pedal_calib_screen_is_fault_active(void) {
  return s_fault_active;
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

StatusCode gui_pedal_calib_widget_subtitle_text(const char *text) {
  if (!s_pedal_calib_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (text == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  return lvgl_widgets_set_label_text(&s_subtitle_label, text);
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

StatusCode gui_pedal_calib_widget_subtitle_text(const char *text) {
  (void)text;
  return STATUS_CODE_OK;
}

StatusCode gui_pedal_calib_screen_set_fault(bool fault_active, uint16_t fault_code, uint8_t cell_at_fault) {
  (void)fault_active;
  (void)fault_code;
  (void)cell_at_fault;
  return STATUS_CODE_OK;
}

bool gui_pedal_calib_screen_is_fault_active(void) {
  return false;
}

#endif
