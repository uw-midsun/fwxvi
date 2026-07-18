/************************************************************************************************
 * @file   gui_fault_screen.c
 *
 * @brief  Full-screen BPS fault takeover screen implementation
 *
 * @date   2026-07-17
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
#include "gui_fault_screen.h"
#include "gui_widgets.h"

#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
#include "lvgl_screens.h"
#include "lvgl_widgets.h"

static lv_obj_t *s_fault_detail_label;
static bool s_fault_widgets_initialized;

static lv_color_t s_gui_palette_color(GuiColorId color_id) {
  ClutEntry color = clut_get_gui_color(color_id);
  return lv_color_make(clut_entry_red(color), clut_entry_green(color), clut_entry_blue(color));
}

StatusCode gui_fault_screen_init(GuiScreen *screen) {
  if (screen == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_fault_widgets_initialized) {
    return STATUS_CODE_ALREADY_INITIALIZED;
  }

  lv_color_t fg_color = s_gui_palette_color(GUI_COLOR_BPS_FAULT_TEXT);

  status_ok_or_return(lvgl_set_background_color(screen, GUI_COLOR_BPS_FAULT_BACKGROUND));

  lv_obj_t *title = lv_label_create(screen);
  if (title == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  lv_label_set_text(title, "BPS FAULT");
  lv_obj_set_style_text_color(title, fg_color, 0);
  lv_obj_set_style_text_font(title, GUI_BIG_TEXT, 0);
  lv_obj_align(title, LV_ALIGN_CENTER, 0, -30);

  s_fault_detail_label = lv_label_create(screen);
  if (s_fault_detail_label == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  lv_label_set_text(s_fault_detail_label, "");
  lv_obj_set_style_text_color(s_fault_detail_label, fg_color, 0);
  lv_obj_set_style_text_font(s_fault_detail_label, GUI_MEDIUM_TEXT, 0);
  lv_obj_set_style_text_align(s_fault_detail_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(s_fault_detail_label, LV_ALIGN_CENTER, 0, 30);

  s_fault_widgets_initialized = true;
  return STATUS_CODE_OK;
}

void gui_fault_screen_deinit(void) {
  s_fault_detail_label = NULL;
  s_fault_widgets_initialized = false;
}

StatusCode gui_fault_screen_widget_set(uint16_t fault_code, uint8_t cell_at_fault) {
  if (!s_fault_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  bool is_cell_fault = false;
  const char *fault_text = gui_widgets_bps_fault_text(fault_code, &is_cell_fault);
  char detail_buffer[LABEL_MAX_CHARS];
  if (is_cell_fault && cell_at_fault != 0U) {
    snprintf(detail_buffer, sizeof(detail_buffer), "%s\nCell %u", fault_text, cell_at_fault);
  } else {
    snprintf(detail_buffer, sizeof(detail_buffer), "%s", fault_text);
  }
  lv_label_set_text(s_fault_detail_label, detail_buffer);

  return STATUS_CODE_OK;
}

#else

StatusCode gui_fault_screen_init(GuiScreen *screen) {
  (void)screen;
  return STATUS_CODE_OK;
}

void gui_fault_screen_deinit(void) {}

StatusCode gui_fault_screen_widget_set(uint16_t fault_code, uint8_t cell_at_fault) {
  (void)fault_code;
  (void)cell_at_fault;
  return STATUS_CODE_OK;
}

#endif
