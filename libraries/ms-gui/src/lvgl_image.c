/************************************************************************************************
 * @file   lvgl_image.c
 *
 * @brief  LVGL-specific image widget wrapper
 *
 * @date   2026-05-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>

/* Inter-component Headers */
#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
#include "clut.h"
#include "lvgl_image.h"
#else
#include "lvgl_image.h"
#endif

/* Intra-component Headers */

#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
static lv_color_t s_gui_palette_color(GuiColorId color_id) {
  ClutEntry color = clut_get_gui_color(color_id);
  return lv_color_make(clut_entry_red(color), clut_entry_green(color), clut_entry_blue(color));
}

static void s_apply_position(lv_obj_t* obj, const WidgetPosition* position) {
  if (obj == NULL || position == NULL) {
    return;
  }

  switch (position->type) {
    case WIDGET_POSITION_ALIGN:
      lv_obj_align(obj, position->value.align.align, position->value.align.x_offset, position->value.align.y_offset);
      break;

    case WIDGET_POSITION_ABSOLUTE:
      lv_obj_set_pos(obj, position->value.absolute.x, position->value.absolute.y);
      break;

    default:
      break;
  }
}

StatusCode lvgl_image_create(LvglImage* image, const LvglImageConfig* config, GuiScreen* parent) {
  if (image == NULL || config == NULL || parent == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  if (config->size.width < 0 || config->size.height < 0) {
    return STATUS_CODE_INVALID_ARGS;
  }

  *image = (LvglImage){ 0 };
  image->image = lv_image_create(parent);
  if (image->image == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  if (config->source != NULL) {
    lv_image_set_src(image->image, config->source);
  }

  lv_obj_set_size(image->image, config->size.width > 0 ? config->size.width : LV_SIZE_CONTENT, config->size.height > 0 ? config->size.height : LV_SIZE_CONTENT);
  s_apply_position(image->image, &config->position);

  if (config->recolor_enabled) {
    status_ok_or_return(lvgl_image_set_recolor(image, config->recolor_color_id));
  }

  return STATUS_CODE_OK;
}

StatusCode lvgl_image_set_source(LvglImage* image, const void* source) {
  if (image == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  if (image->image == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  lv_image_set_src(image->image, source);
  return STATUS_CODE_OK;
}

StatusCode lvgl_image_set_recolor(LvglImage* image, GuiColorId color_id) {
  if (image == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  if (image->image == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  lv_obj_set_style_image_recolor(image->image, s_gui_palette_color(color_id), LV_PART_MAIN);
  lv_obj_set_style_image_recolor_opa(image->image, LV_OPA_COVER, LV_PART_MAIN);
  return STATUS_CODE_OK;
}

StatusCode lvgl_image_clear_recolor(LvglImage* image) {
  if (image == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  if (image->image == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  lv_obj_set_style_image_recolor_opa(image->image, LV_OPA_TRANSP, LV_PART_MAIN);
  return STATUS_CODE_OK;
}

#else
StatusCode lvgl_image_create(LvglImage* image, const LvglImageConfig* config, GuiScreen* parent) {
  (void)config;
  (void)parent;
  if (image != NULL) {
    *image = (LvglImage){ 0 };
  }
  return STATUS_CODE_OK;
}

StatusCode lvgl_image_set_source(LvglImage* image, const void* source) {
  (void)image;
  (void)source;
  return STATUS_CODE_OK;
}

StatusCode lvgl_image_set_recolor(LvglImage* image, GuiColorId color_id) {
  (void)image;
  (void)color_id;
  return STATUS_CODE_OK;
}

StatusCode lvgl_image_clear_recolor(LvglImage* image) {
  (void)image;
  return STATUS_CODE_OK;
}
#endif
