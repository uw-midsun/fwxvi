/************************************************************************************************
 * @file   lvgl_widgets.c
 *
 * @brief  LVGL-specific widgets used by gui_widgets
 *
 * @date   2026-03-13
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Intra-component Headers */
#include "clut.h"
#include "lvgl_screens.h"


static lv_color_t s_gui_palette_color(GuiColorId color_id) {
  ClutEntry color = clut_get_gui_color(color_id);
  return lv_color_make(clut_entry_red(color), clut_entry_green(color), clut_entry_blue(color));
}

StatusCode lvgl_set_background_color(GuiScreen *screen, GuiColorId screen_color) {
  if (screen == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  lv_obj_set_style_bg_color(screen, s_gui_palette_color(screen_color), 0);

  return STATUS_CODE_OK;
}

GuiScreen *lvgl_get_active_screen() {
  return lv_screen_active();
}
