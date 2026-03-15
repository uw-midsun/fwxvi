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
#include "status.h"

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

typedef lv_align_t WidgetAlignment;
typedef lv_bar_orientation_t WidgetOrientation;

/** @brief Store widget size properties */
typedef struct {
  lv_coord_t width;  /**< Widget width in pixels */
  lv_coord_t height; /**< Widget height in pixels */
} WidgetSize;

/** @brief Supported widget position modes */
typedef enum {
  WIDGET_POSITION_ALIGN,    /**< If we want widget position to be specified by a WIDGET_ALIGN macro + Offset */
  WIDGET_POSITION_ABSOLUTE, /**< If we want coordinates for our position */
} WidgetPositionType;

/** @brief Struct storing the aligned position + Offset */
typedef struct {
  WidgetAlignment align; /**< LVGL alignment anchor used to place the widget */
  lv_coord_t x_offset;   /**< Horizontal offset from the alignment anchor */
  lv_coord_t y_offset;   /**< Vertical offset from the alignment anchor */
} WidgetAlignedPosition;

/** @brief Struct specifying the absolute position of the widget */
typedef struct {
  lv_coord_t x;   /**< Absolute X position in pixels */
  lv_coord_t y;   /**< Absolute Y position in pixels */
} WidgetAbsolutePosition;

/** @brief Union struct which describes the position of the widget */
typedef struct {
  WidgetPositionType type;           /**< Position mode used to interpret the value union */
  union {
    WidgetAlignedPosition align;     /**< Alignment-based position configuration */
    WidgetAbsolutePosition absolute; /**< Absolute coordinate position configuration */
  } value;                           /**< Position data for the selected mode */
} WidgetPosition;

/** @brief Struct storing runtime info needed for the speedometer widget */
typedef struct {
  lv_obj_t *scale;                      /**< LVGL scale object representing the speedometer dial */
  lv_obj_t *needle;                     /**< LVGL line object used as the speedometer needle */
  lv_obj_t *label;                      /**< LVGL label object showing the numeric speed */
  lv_point_precise_t needle_points[2];  /**< Cached line points for the needle geometry */
  int32_t min_value;                    /**< Minimum speed value supported by the widget */
  int32_t max_value;                    /**< Maximum speed value supported by the widget */
  int32_t angle_range;                  /**< Angular sweep of the speedometer dial */
  int32_t rotation;                     /**< Starting rotation angle of the speedometer dial */
} SpeedometerWidget;

/** @brief Struct storing the config needed to initially draw the speedometer widget */
typedef struct {
  WidgetSize size;             /**< Overall size of the speedometer widget */
  WidgetPosition position;     /**< Placement of the speedometer within its parent */
  uint16_t total_tick_count;   /**< Total number of ticks drawn around the scale */
  uint16_t major_tick_every;   /**< Interval between major ticks */
  int32_t angle_range;         /**< Angular sweep of the speedometer dial */
  int32_t rotation;            /**< Starting rotation angle of the speedometer dial */
  int32_t needle_length;       /**< Needle length passed to the LVGL scale helper */
} SpeedometerWidgetConfig;

/** @brief Struct storing runtime info needed for the bar widget */
typedef struct {
  lv_obj_t *bar;   /**< LVGL bar object representing the indicator fill */
  lv_obj_t *label; /**< LVGL label object associated with the bar */
} BarWidget;

/** @brief Configuration used to create a bar widget */
typedef struct {
  WidgetSize size;                      /**< Overall size of the bar widget */
  WidgetPosition position;              /**< Placement of the bar within its parent */
  const char *label_text;               /**< Static text displayed beside the bar */
  WidgetAlignment label_text_alignment; /**< Alignment used to place the label relative to the bar */
  WidgetOrientation orientation;        /**< Orientation of the LVGL bar widget */
  GuiColorId indicator_color_id;        /**< Semantic palette color used for the indicator fill */
} BarWidgetConfig;

/* Inside alignment (will be inside object) */
#define WIDGET_ALIGN_IN_TOP_LEFT      LV_ALIGN_TOP_LEFT
#define WIDGET_ALIGN_IN_TOP_MID       LV_ALIGN_TOP_MID
#define WIDGET_ALIGN_IN_TOP_RIGHT     LV_ALIGN_TOP_RIGHT
#define WIDGET_ALIGN_IN_BOTTOM_LEFT   LV_ALIGN_BOTTOM_LEFT
#define WIDGET_ALIGN_IN_BOTTOM_MID    LV_ALIGN_BOTTOM_MID
#define WIDGET_ALIGN_IN_BOTTOM_RIGHT  LV_ALIGN_BOTTOM_RIGHT
#define WIDGET_ALIGN_IN_LEFT_MID      LV_ALIGN_LEFT_MID
#define WIDGET_ALIGN_IN_RIGHT_MID     LV_ALIGN_RIGHT_MID
#define WIDGET_ALIGN_CENTER           LV_ALIGN_CENTER

/* Outside alignment (will be Outside object) */
#define WIDGET_ALIGN_OUT_TOP_LEFT     LV_ALIGN_OUT_TOP_LEFT
#define WIDGET_ALIGN_OUT_TOP_MID      LV_ALIGN_OUT_TOP_MID
#define WIDGET_ALIGN_OUT_TOP_RIGHT    LV_ALIGN_OUT_TOP_RIGHT
#define WIDGET_ALIGN_OUT_BOTTOM_LEFT  LV_ALIGN_OUT_BOTTOM_LEFT
#define WIDGET_ALIGN_OUT_BOTTOM_MID   LV_ALIGN_OUT_BOTTOM_MID
#define WIDGET_ALIGN_OUT_BOTTOM_RIGHT LV_ALIGN_OUT_BOTTOM_RIGHT
#define WIDGET_ALIGN_OUT_LEFT_TOP     LV_ALIGN_OUT_LEFT_TOP
#define WIDGET_ALIGN_OUT_LEFT_MID     LV_ALIGN_OUT_LEFT_MID
#define WIDGET_ALIGN_OUT_LEFT_BOTTOM  LV_ALIGN_OUT_LEFT_BOTTOM
#define WIDGET_ALIGN_OUT_RIGHT_TOP    LV_ALIGN_OUT_RIGHT_TOP
#define WIDGET_ALIGN_OUT_RIGHT_MID    LV_ALIGN_OUT_RIGHT_MID
#define WIDGET_ALIGN_OUT_RIGHT_BOTTOM LV_ALIGN_OUT_RIGHT_BOTTOM

/* Widget orientation */
#define WIDGET_ORIENTATION_AUTO       LV_BAR_ORIENTATION_AUTO
#define WIDGET_ORIENTATION_HORIZONTAL LV_BAR_ORIENTATION_HORIZONTAL
#define WIDGET_ORIENTATION_VERTICAL   LV_BAR_ORIENTATION_VERTICAL

/* Fonts */
#define GUI_SMALL_TEXT &lv_font_montserrat_14
#define GUI_MEDIUM_TEXT &lv_font_montserrat_26
#define GUI_BIG_TEXT &lv_font_montserrat_40

/* Min/Max values for various widgets */
#define SPEEDOMETER_MIN_VALUE 0
#define SPEEDOMETER_MAX_VALUE 200

#define BAR_MIN_VALUE 0
#define BAR_MAX_VALUE 100

/**
 * @brief   Create and initialize a speedometer widget
 * @param   speedometer Pointer to runtime storage for the widget
 * @param   config Pointer to the initial speedometer configuration
 * @param   parent Parent LVGL object that owns the widget
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lvgl_widgets_create_speedometer(SpeedometerWidget *speedometer, const SpeedometerWidgetConfig *config, lv_obj_t *parent);

/**
 * @brief   Update the displayed speedometer value
 * @param   speedometer Pointer to the initialized speedometer widget
 * @param   speed_kmh Speed value in km/h
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lvgl_widgets_set_speed(SpeedometerWidget *speedometer, float speed_kmh);

/**
 * @brief   Create and initialize a bar widget
 * @param   bar_widget Pointer to runtime storage for the widget
 * @param   config Pointer to the initial bar configuration
 * @param   parent Parent LVGL object that owns the widget
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lvgl_widgets_create_bar(BarWidget *bar_widget, const BarWidgetConfig *config, lv_obj_t *parent);

/**
 * @brief   Update the displayed bar value
 * @param   bar_widget Pointer to the initialized bar widget
 * @param   value New bar value
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lvgl_widgets_set_bar_value(BarWidget *bar_widget, int32_t value);

/** @} */
