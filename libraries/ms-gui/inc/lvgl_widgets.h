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
#include "lvgl.h"
#include "status.h"

/* Intra-component Headers */
#include "clut.h"
#include "lvgl_screens.h"

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

typedef lv_align_t WidgetAlignment;
typedef lv_bar_orientation_t WidgetOrientation;

/** @brief   Struct defining the dimensions of a widget */
typedef struct {
  lv_coord_t width;  /**< Widget width in pixels */
  lv_coord_t height; /**< Widget height in pixels */
} WidgetSize;

/** @brief   Positioning modes supported by widgets */
typedef enum {
  WIDGET_POSITION_ALIGN,    /**< Position the widget relative to an LVGL alignment anchor */
  WIDGET_POSITION_ABSOLUTE, /**< Position the widget using an absolute (x,y) coords */
} WidgetPositionType;

/** @brief   Position a widget using an LVGL alignment and offsets */
typedef struct {
  WidgetAlignment align; /**< LVGL alignment anchor used for placement */
  lv_coord_t x_offset;   /**< Horizontal offset from the alignment anchor */
  lv_coord_t y_offset;   /**< Vertical offset from the alignment anchor */
} WidgetAlignedPosition;

/** @brief   Position a widget using absolute screen coordinates */
typedef struct {
  lv_coord_t x; /**< Absolute X coordinate in pixels */
  lv_coord_t y; /**< Absolute Y coordinate in pixels */
} WidgetAbsolutePosition;

/** @brief   Generic widget position description */
typedef struct {
  WidgetPositionType type; /**< Positioning mode used by the union below */
  union {
    WidgetAlignedPosition align;     /**< Alignment-based position configuration */
    WidgetAbsolutePosition absolute; /**< Absolute position configuration */
  } value;                           /**< Position data matching the selected mode */
} WidgetPosition;

/** @brief   Runtime handles and state for a speedometer widget */
typedef struct {
  lv_obj_t *scale;                     /**< LVGL scale object used for the dial */
  lv_obj_t *needle;                    /**< LVGL line object used as the speedometer needle */
  lv_obj_t *label;                     /**< LVGL label object showing the current speed */
  lv_point_precise_t needle_points[2]; /**< Line endpoints used to draw the needle */
} SpeedometerWidget;

/** @brief   Configuration used when creating a speedometer widget */
typedef struct {
  WidgetSize size;           /**< Bounding size of the widget */
  WidgetPosition position;   /**< Placement of the widget on screen */
  uint16_t total_tick_count; /**< Total number of dial ticks to draw */
  uint16_t major_tick_every; /**< Interval between emphasized major ticks */
  int32_t angle_range;       /**< Sweep angle of the dial in degrees */
  int32_t rotation;          /**< Rotation offset applied to the dial in degrees */
  int32_t needle_length;     /**< Needle length in pixels */
} SpeedometerWidgetConfig;

/** @brief   Runtime handles for a labeled bar widget */
typedef struct {
  lv_obj_t *bar;            /**< LVGL bar object */
  lv_obj_t *label;          /**< LVGL label associated with the bar */
} BarWidget;

/** @brief   Configuration used when creating a labeled bar widget */
typedef struct {
  WidgetSize size;                      /**< Bounding size of the bar widget */
  WidgetPosition position;              /**< Placement of the bar widget on screen */
  const char *label_text;               /**< Static label text shown beside the bar */
  WidgetAlignment label_text_alignment; /**< Alignment used for the label relative to the bar */
  WidgetOrientation orientation;        /**< LVGL bar orientation */
  GuiColorId indicator_color_id;        /**< Ccolor used for the bar indicator */
} BarWidgetConfig;

/* Inside alignment (inside parent widget) */
#define WIDGET_ALIGN_IN_TOP_LEFT LV_ALIGN_TOP_LEFT
#define WIDGET_ALIGN_IN_TOP_MID LV_ALIGN_TOP_MID
#define WIDGET_ALIGN_IN_TOP_RIGHT LV_ALIGN_TOP_RIGHT
#define WIDGET_ALIGN_IN_BOTTOM_LEFT LV_ALIGN_BOTTOM_LEFT
#define WIDGET_ALIGN_IN_BOTTOM_MID LV_ALIGN_BOTTOM_MID
#define WIDGET_ALIGN_IN_BOTTOM_RIGHT LV_ALIGN_BOTTOM_RIGHT
#define WIDGET_ALIGN_IN_LEFT_MID LV_ALIGN_LEFT_MID
#define WIDGET_ALIGN_IN_RIGHT_MID LV_ALIGN_RIGHT_MID
#define WIDGET_ALIGN_CENTER LV_ALIGN_CENTER

/* Outside alighment (outside of parent widget) */
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

/* Orientation (Other widgets besides bars use this param) */
#define WIDGET_ORIENTATION_AUTO LV_BAR_ORIENTATION_AUTO
#define WIDGET_ORIENTATION_HORIZONTAL LV_BAR_ORIENTATION_HORIZONTAL
#define WIDGET_ORIENTATION_VERTICAL LV_BAR_ORIENTATION_VERTICAL

/* Fonts */
#define GUI_SMALL_TEXT &lv_font_montserrat_14
#define GUI_MEDIUM_TEXT &lv_font_montserrat_26
#define GUI_BIG_TEXT &lv_font_montserrat_40

/* Speedometer widget min and max vals */
#define SPEEDOMETER_MIN_VALUE 0
#define SPEEDOMETER_MAX_VALUE 200

/* Bar widget min and max vals */
#define BAR_MIN_VALUE 0
#define BAR_MAX_VALUE 100

/**
 * @brief   Create and initialize a speedometer widget
 * @param   speedometer Pointer to the runtime speedometer object to initialize
 * @param   config Pointer to the widget configuration
 * @param   parent Parent LVGL object that will own the widget
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lvgl_widgets_create_speedometer(SpeedometerWidget *speedometer, const SpeedometerWidgetConfig *config, lv_obj_t *parent);

/**
 * @brief   Update the displayed speed on a speedometer widget
 * @param   speedometer Pointer to the speedometer widget runtime object
 * @param   speed_kmh Current speed in km/h
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lvgl_widgets_set_speed(SpeedometerWidget *speedometer, float speed_kmh);

/**
 * @brief   Create and initialize a labeled bar widget
 * @param   bar_widget Pointer to the runtime bar widget object to initialize
 * @param   config Pointer to the widget configuration
 * @param   parent Parent LVGL object that will own the widget
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lvgl_widgets_create_bar(BarWidget *bar_widget, const BarWidgetConfig *config, lv_obj_t *parent);

/**
 * @brief   Update the current value of a bar widget
 * @param   bar_widget Pointer to the bar widget runtime object
 * @param   value Current bar value
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lvgl_widgets_set_bar_value(BarWidget *bar_widget, int32_t value);

/** @} */
