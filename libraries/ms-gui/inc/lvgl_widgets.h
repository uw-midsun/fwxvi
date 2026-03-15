#pragma once

/************************************************************************************************
 * @file   lvgl_widgets.h
 *
 * @brief  LVGL-specific widgets used by gui_widgets
 *
 * @date   2026-03-13
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "clut.h"
#include "lvgl.h"

/* Intra-component Headers */
#include "status.h"

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

typedef lv_obj_t GuiScreen;

#define SPEEDOMETER_MIN_VALUE 0
#define SPEEDOMETER_MAX_VALUE 200
#define BAR_MIN_VALUE 0
#define BAR_MAX_VALUE 100

typedef struct {
  lv_coord_t width;
  lv_coord_t height;
} WidgetSize;

typedef enum {
  WIDGET_POSITION_ALIGN,
  WIDGET_POSITION_ABSOLUTE,
} WidgetPositionType;

typedef lv_align_t WidgetAlignment;
typedef lv_bar_orientation_t WidgetOrientation;

#define WIDGET_ALIGN_IN_TOP_LEFT LV_ALIGN_TOP_LEFT
#define WIDGET_ALIGN_IN_TOP_MID LV_ALIGN_TOP_MID
#define WIDGET_ALIGN_IN_TOP_RIGHT LV_ALIGN_TOP_RIGHT
#define WIDGET_ALIGN_IN_BOTTOM_LEFT LV_ALIGN_BOTTOM_LEFT
#define WIDGET_ALIGN_IN_BOTTOM_MID LV_ALIGN_BOTTOM_MID
#define WIDGET_ALIGN_IN_BOTTOM_RIGHT LV_ALIGN_BOTTOM_RIGHT
#define WIDGET_ALIGN_IN_LEFT_MID LV_ALIGN_LEFT_MID
#define WIDGET_ALIGN_IN_RIGHT_MID LV_ALIGN_RIGHT_MID
#define WIDGET_ALIGN_CENTER LV_ALIGN_CENTER

#define WIDGET_ALIGN_OUT_TOP_LEFT LV_ALIGN_OUT_TOP_LEFT
#define WIDGET_ALIGN_OUT_TOP_MID LV_ALIGN_OUT_TOP_MID
#define WIDGET_ALIGN_OUT_TOP_RIGHT LV_ALIGN_OUT_TOP_RIGHT
#define WIDGET_ALIGN_OUT_BOTTOM_LEFT LV_ALIGN_OUT_BOTTOM_LEFT
#define WIDGET_ALIGN_OUT_BOTTOM_MID LV_ALIGN_OUT_BOTTOM_MID
#define WIDGET_ALIGN_OUT_BOTTOM_RIGHT LV_ALIGN_OUT_BOTTOM_RIGHT
#define WIDGET_ALIGN_OUT_LEFT_TOP LV_ALIGN_OUT_LEFT_TOP
#define WIDGET_ALIGN_OUT_LEFT_MID LV_ALIGN_OUT_LEFT_MID
#define WIDGET_ALIGN_OUT_LEFT_BOTTOM LV_ALIGN_OUT_LEFT_BOTTOM
#define WIDGET_ALIGN_OUT_RIGHT_TOP LV_ALIGN_OUT_RIGHT_TOP
#define WIDGET_ALIGN_OUT_RIGHT_MID LV_ALIGN_OUT_RIGHT_MID
#define WIDGET_ALIGN_OUT_RIGHT_BOTTOM LV_ALIGN_OUT_RIGHT_BOTTOM

#define WIDGET_ORIENTATION_AUTO LV_BAR_ORIENTATION_AUTO
#define WIDGET_ORIENTATION_HORIZONTAL LV_BAR_ORIENTATION_HORIZONTAL
#define WIDGET_ORIENTATION_VERTICAL LV_BAR_ORIENTATION_VERTICAL

typedef struct {
  WidgetAlignment align;
  lv_coord_t x_offset;
  lv_coord_t y_offset;
} WidgetAlignedPosition;

typedef struct {
  lv_coord_t x;
  lv_coord_t y;
} WidgetAbsolutePosition;

typedef struct {
  WidgetPositionType type;
  union {
    WidgetAlignedPosition align;
    WidgetAbsolutePosition absolute;
  } value;
} WidgetPosition;

typedef struct {
  lv_obj_t *scale;
  lv_obj_t *needle;
  lv_obj_t *label;
  lv_point_precise_t needle_points[2];
  int32_t min_value;
  int32_t max_value;
  int32_t angle_range;
  int32_t rotation;
} SpeedometerWidget;

typedef struct {
  WidgetSize size;
  WidgetPosition position;
  uint16_t total_tick_count;
  uint16_t major_tick_every;
  int32_t angle_range;
  int32_t rotation;
  int32_t needle_length;
} SpeedometerWidgetConfig;

typedef struct {
  lv_obj_t *bar;
  lv_obj_t *label;
} BarWidget;

typedef struct {
  WidgetSize size;
  WidgetPosition position;
  const char *label_text;
  WidgetAlignment label_text_alignment;
  WidgetOrientation orientation;
  GuiColorId indicator_color_id;
} BarWidgetConfig;

/* Fonts */
#define GUI_SMALL_TEXT &lv_font_montserrat_14
#define GUI_MEDIUM_TEXT &lv_font_montserrat_26
#define GUI_BIG_TEXT &lv_font_montserrat_40

StatusCode lvgl_widgets_create_speedometer(SpeedometerWidget *speedometer, const SpeedometerWidgetConfig *config, lv_obj_t *parent);
StatusCode lvgl_widgets_set_speed(SpeedometerWidget *speedometer, float speed_kmh);

StatusCode lvgl_widgets_create_bar(BarWidget *bar_widget, const BarWidgetConfig *config, lv_obj_t *parent);
StatusCode lvgl_widgets_set_bar_value(BarWidget *bar_widget, int32_t value);

/** @} */
