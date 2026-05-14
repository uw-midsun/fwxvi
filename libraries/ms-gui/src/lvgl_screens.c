/************************************************************************************************
 * @file   lvgl_screens.c
 *
 * @brief  LVGL-specific screen helpers used by gui_* layers
 *
 * @date   2026-03-13
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>

/* Inter-component Headers */
#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
#include "clut.h"
#include "lvgl_screens.h"
#else
#include "lvgl_screens.h"
#endif

/* Intra-component Headers */

#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
/**
 * @brief   Convert a semantic GUI color ID to an LVGL color
 * @param   color_id Semantic color identifier
 * @return  LVGL color matching the requested palette entry
 */
static lv_color_t s_gui_palette_color(GuiColorId color_id) {
  ClutEntry color = clut_get_gui_color(color_id);
  return lv_color_make(clut_entry_red(color), clut_entry_green(color), clut_entry_blue(color));
}

StatusCode lvgl_set_background_color(GuiScreen *screen, GuiColorId screen_color) {
  if (screen == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  lv_obj_set_style_bg_color(screen, s_gui_palette_color(screen_color), 0);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

  return STATUS_CODE_OK;
}

StatusCode lvgl_screens_create(GuiScreen **screen) {
  if (screen == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  *screen = lv_obj_create(NULL);
  if (*screen == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  lv_obj_clear_flag(*screen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scrollbar_mode(*screen, LV_SCROLLBAR_MODE_OFF);
  return STATUS_CODE_OK;
}

StatusCode lvgl_screens_load(GuiScreen *screen) {
  if (screen == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  lv_screen_load(screen);
  return STATUS_CODE_OK;
}

StatusCode lvgl_screens_destroy(GuiScreen *screen) {
  if (screen == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  lv_obj_delete(screen);
  return STATUS_CODE_OK;
}

StatusCode lvgl_screens_clean(GuiScreen *screen) {
  if (screen == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  lv_obj_clean(screen);
  return STATUS_CODE_OK;
}

GuiScreen *lvgl_get_active_screen() {
  return lv_screen_active();
}

GuiScreen *lvgl_get_top_layer() {
  return lv_layer_top();
}
#else
StatusCode lvgl_set_background_color(GuiScreen *screen, GuiColorId screen_color) {
  (void)screen;
  (void)screen_color;
  return STATUS_CODE_OK;
}

StatusCode lvgl_screens_create(GuiScreen **screen) {
  if (screen == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  *screen = (GuiScreen *)0;
  return STATUS_CODE_OK;
}

StatusCode lvgl_screens_load(GuiScreen *screen) {
  (void)screen;
  return STATUS_CODE_OK;
}

StatusCode lvgl_screens_destroy(GuiScreen *screen) {
  (void)screen;
  return STATUS_CODE_OK;
}

StatusCode lvgl_screens_clean(GuiScreen *screen) {
  (void)screen;
  return STATUS_CODE_OK;
}

GuiScreen *lvgl_get_active_screen() {
  return (GuiScreen *)0;
}

GuiScreen *lvgl_get_top_layer() {
  return (GuiScreen *)0;
}
#endif
