#pragma once

/************************************************************************************************
 * @file   generic_widget.h
 *
 * @brief  A generic LVGL widget wrapper
 *
 * @date   2026-05-12
 * @author Midnight Sun Team #24 - MSXVI
 ********
 */

/* Standard library Headers */

/* Inter-component Headers */
#include "lvgl.h"

/* Intra-component Headers */

/**
 * @defgroup generic_wrapper
 * @brief    generic_wrapper Firmware
 * @{
 */

/**
 * @brief Configuration struct for a generic LVGL widget.
 * All settings for the widget are defined here.
 */
typedef struct {
  int32_t x;
  int32_t y;
  int32_t width;
  int32_t height;
  const lv_image_dsc_t *img_src;
  lv_obj_t *parent;
  lv_color_t GuiColorId;
} WidgetConfig;

/**
 * @brief Represents a generic widget instance.
 */
typedef struct {
  lv_obj_t *obj;
  WidgetConfig *config;
} WidgetInstance;

/**
 * @brief Create a wrapper object from configuration
 */
WidgetInstance create_widget(WidgetConfig *config);

/**
 * @brief Set the image inside widget object
 */
void set_image(const lv_image_dsc_t *img, WidgetInstance *widget);

/**
 * @brief Set the color inside widget object
 */
void set_color(const lv_color_t color, WidgetInstance *widget);

/** @} */
