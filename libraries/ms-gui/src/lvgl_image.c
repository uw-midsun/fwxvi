/************************************************************************************************
 * @file   generic_widget.h
 *
 * @brief  A generic LVGL widget wrapper
 *
 * @date   2026-05-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "generic_wrapper.h"

/* Intra-component Headers */

/**
 * @brief   Create an instance of wrapper from Widget config
 * @param   config Widget configuration
 * @return  Return widget object, with new object and lvgl image configured accordingly
 */
WidgetInstance *create_widget(WidgetConfig *config) {
  if (!config) return NULL;

  WidgetInstance *widget = malloc(sizeof(WidgetInstance));
  if (!widget) return NULL;

  widget->config = config;
  widget->obj = lv_image_create(config->parent);
  if (!widget->obj) {
    free(widget);
    return NULL;
  }

  lv_obj_set_pos(widget->obj, config->x, config->y);
  lv_obj_set_size(widget->obj, config->width, config->height);

  if (config->img_src) {
    lv_image_set_src(widget->obj, config->img_src);
  }

  lv_obj_set_style_bg_color(widget->obj, config->GuiColorId, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(widget->obj, LV_OPA_COVER, LV_PART_MAIN);

  return widget;
}

/**
 * @brief   Chnage the image object of widget
 * @param   widget Widget instance
 * @param   img Lvgl image
 * @return  None
 */
void set_image(const lv_image_dsc_t *img, WidgetInstance *widget) {
  if (widget->obj && widget) {
    lv_image_set_src(widget->obj, img);
  }
}

/**
 * @brief   Chnage the color of widget's image
 * @param   widget Widget instance
 * @param   color Lvgl color
 * @return  None
 */
void set_color(const lv_color_t color, WidgetInstance *widget) {
  if (widget && widget->obj) {
    lv_obj_set_style_bg_color(widget->obj, color, LV_PART_MAIN);
  }
}
