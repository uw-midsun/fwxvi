/************************************************************************************************
 * @file   gui_display_brightness_screen.c
 *
 * @brief  High-level display brightness screen implementation
 *
 * @date   2026-11-10
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

#include "status.h"

/* Inter-component Headers */
#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
#include "lvgl.h"
#endif

/* Intra-component Headers */
#include "clut.h"
#include "display_defs.h"
#include "gui_therm_screen.h"

#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
#include "lvgl_screens.h"
#include "lvgl_widgets.h"

static bool s_display_brightness_widgets_initialized;

static StatusCode s_create_title_label(GuiScreen *screen) {
  const LabelWidgetConfig title_config = {
    .size = { .width = DISPLAY_WIDTH, .height = 20 },
    .position = { .type = WIDGET_POSITION_ALIGN, .value.align = { .align = WIDGET_ALIGN_IN_TOP_MID, .x_offset = 0, .y_offset = 4 } },
    .label_text = "Display Brightness",
    .alignment = WIDGET_TEXT_ALIGN_CENTER,
    .text_color_id = GUI_COLOR_TEXT_PRIMARY,
    .font = GUI_SMALL_TEXT,
    .background_enabled = false,
    .background_color_id = 0,
    .border_enabled = false,
    .border_color_id = GUI_COLOR_LABEL_BORDER,
    .border_width = 0,
  };

  static LabelWidget s_title_label;
  return lvgl_widgets_create_label(&s_title_label, &title_config, screen);
}

StatusCode gui_display_brightness_screen_init(GuiScreen *screen) {
  if (screen == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_display_brightness_widgets_initialized) {
    return STATUS_CODE_ALREADY_INITIALIZED;
  }

  status_ok_or_return(lvgl_set_background_color(screen, GUI_COLOR_SCREEN_BACKGROUND));

  status_ok_or_return(s_create_title_label(screen));

  s_display_brightness_widgets_initialized = true;
  return STATUS_CODE_OK;
}

void gui_display_brightness_screen_deinit(void) {
  //   s_therm_table = (TableWidget){ 0 };
  s_display_brightness_widgets_initialized = false;
}

#else

StatusCode gui_display_brightness_screen_init(GuiScreen *screen) {
  (void)screen;
  return STATUS_CODE_OK;
}

void gui_display_brightness_screen_deinit(void) {}

#endif
