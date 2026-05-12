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


#include "lvgl.h"

/**
 * @brief Configuration struct for a generic LVGL widget.
 * All settings for the widget are defined here.
 */
typedef struct{
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    const lv_image_dsc_t *img_src;
    lv_obj_t *parent;
    lv_color_t color;
}widget_config_t;

/**
 * @brief Represents a generic widget instance.
 */
typedef struct{
    lv_obj_t *obj;
    widget_config_t* config;
}widget_t;

widget_t create_widget(widget_config_t* config);

void set_image(const lv_image_dsc_t* img, widget_t *widget);

void set_color(const lv_color_t color, widget_t *widget);



