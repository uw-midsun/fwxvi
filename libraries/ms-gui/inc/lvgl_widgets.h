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
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#ifdef STM32L4P5xx
#include "lvgl.h"
#endif
#include "status.h"

/* Intra-component Headers */
#include "clut.h"
#include "lvgl_screens.h"
#include "display_defs.h"

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */
#ifdef STM32L4P5xx
typedef lv_align_t WidgetAlignment;
typedef lv_bar_orientation_t WidgetOrientation;
typedef const lv_font_t *WidgetFontSize;

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

/** 
 * @brief  Alignment of text 
 * @note   We start at 1 to match lv_text_align_t */
typedef enum {
  WIDGET_TEXT_ALIGN_LEFT = 1,     /**< Align text to left*/
  WIDGET_TEXT_ALIGN_CENTER,       /**< Align text to center*/
  WIDGET_TEXT_ALIGN_RIGHT,        /**< Align text to right*/
} WidgetTextAlignment;

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
  lv_obj_t *bar;   /**< LVGL bar object */
  lv_obj_t *label; /**< LVGL label associated with the bar */
} BarWidget;

/** @brief   Configuration used when creating a labeled bar widget */
typedef struct {
  WidgetSize size;                      /**< Bounding size of the bar widget */
  WidgetPosition position;              /**< Placement of the bar widget on screen */
  const char *label_text;               /**< Static label text shown beside the bar */
  WidgetAlignment label_alignment; /**< Alignment used for the label relative to the bar */
  WidgetOrientation orientation;        /**< LVGL bar orientation */
  GuiColorId indicator_color_id;        /**< Ccolor used for the bar indicator */
} BarWidgetConfig;

typedef struct {
  lv_obj_t *label;
} LabelWidget;

typedef struct {
  WidgetSize size;                 /**< Bounding size of the label widget, or content-sized when zero */
  WidgetPosition position;         /**< Placement of the label widget on screen */
  const char *label_text;          /**< Static label text shown */
  WidgetTextAlignment alignment;   /**< Text alignment inside the label */
  GuiColorId text_color_id;        /**< Color used for the label text */
  WidgetFontSize font;             /**< Font used by the label text */
  bool background_enabled;         /**< Whether to draw a background behind the label */
  GuiColorId background_color_id;  /**< Color used for the background when enabled */
  bool border_enabled;             /**< Whether to draw a border around the label */
  GuiColorId border_color_id;      /**< Color used for the border when enabled */
  lv_coord_t border_width;         /**< Border width in pixels when the border is enabled */
  /* TODO: Maybe add in a WidgetsFont member when FW-506 is done */
} LabelWidgetConfig;

#else
typedef int16_t WidgetAlignment;
typedef int16_t WidgetOrientation;
typedef const int16_t *WidgetFontSize;

/** @brief   Struct defining the dimensions of a widget */
typedef struct {
  int16_t width;  /**< Widget width in pixels */
  int16_t height; /**< Widget height in pixels */
} WidgetSize;

/** @brief   Positioning modes supported by widgets */
typedef enum {
  WIDGET_POSITION_ALIGN,    /**< Position the widget relative to an LVGL alignment anchor */
  WIDGET_POSITION_ABSOLUTE, /**< Position the widget using an absolute (x,y) coords */
} WidgetPositionType;

/** 
 * @brief  Alignment of text 
 * @note   We start at 1 to match lv_text_align_t */
typedef enum {
  WIDGET_TEXT_ALIGN_LEFT = 1,     /**< Align text to left*/
  WIDGET_TEXT_ALIGN_CENTER,       /**< Align text to center*/
  WIDGET_TEXT_ALIGN_RIGHT,        /**< Align text to right*/
} WidgetTextAlignment;

/** @brief   Position a widget using an LVGL alignment and offsets */
typedef struct {
  WidgetAlignment align; /**< LVGL alignment anchor used for placement */
  int16_t x_offset;   /**< Horizontal offset from the alignment anchor */
  int16_t y_offset;   /**< Vertical offset from the alignment anchor */
} WidgetAlignedPosition;

/** @brief   Position a widget using absolute screen coordinates */
typedef struct {
  int16_t x; /**< Absolute X coordinate in pixels */
  int16_t y; /**< Absolute Y coordinate in pixels */
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
  int16_t *scale;                     /**< LVGL scale object used for the dial */
  int16_t *needle;                    /**< LVGL line object used as the speedometer needle */
  int16_t *label;                     /**< LVGL label object showing the current speed */
  int16_t needle_points[2]; /**< Line endpoints used to draw the needle */
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
  int16_t *bar;   /**< LVGL bar object */
  int16_t *label; /**< LVGL label associated with the bar */
} BarWidget;

/** @brief   Configuration used when creating a labeled bar widget */
typedef struct {
  WidgetSize size;                      /**< Bounding size of the bar widget */
  WidgetPosition position;              /**< Placement of the bar widget on screen */
  const char *label_text;               /**< Static label text shown beside the bar */
  WidgetAlignment label_alignment; /**< Alignment used for the label relative to the bar */
  WidgetOrientation orientation;        /**< LVGL bar orientation */
  GuiColorId indicator_color_id;        /**< Ccolor used for the bar indicator */
} BarWidgetConfig;

typedef struct {
  int16_t *label;
} LabelWidget;

typedef struct {
  WidgetSize size;                 /**< Bounding size of the label widget, or content-sized when zero */
  WidgetPosition position;         /**< Placement of the label widget on screen */
  const char *label_text;          /**< Static label text shown */
  WidgetTextAlignment alignment;   /**< Text alignment inside the label */
  GuiColorId text_color_id;        /**< Color used for the label text */
  WidgetFontSize font;             /**< Font used by the label text */
  bool background_enabled;         /**< Whether to draw a background behind the label */
  GuiColorId background_color_id;  /**< Color used for the background when enabled */
  bool border_enabled;             /**< Whether to draw a border around the label */
  GuiColorId border_color_id;      /**< Color used for the border when enabled */
  int16_t border_width;           /**< Border width in pixels when the border is enabled */
  /* TODO: Maybe add in a WidgetsFont member when FW-506 is done */
} LabelWidgetConfig;

#endif

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

StatusCode lvgl_widgets_create_label(LabelWidget *label, const LabelWidgetConfig *config, GuiScreen *parent);
StatusCode lvgl_widgets_set_label_text(LabelWidget *label, const char *text);

/**
 * @brief   Create and initialize a speedometer widget
 * @param   speedometer Pointer to the runtime speedometer object to initialize
 * @param   config Pointer to the widget configuration
 * @param   parent Parent LVGL object that will own the widget
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lvgl_widgets_create_speedometer(SpeedometerWidget *speedometer, const SpeedometerWidgetConfig *config, GuiScreen *parent);

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
StatusCode lvgl_widgets_create_bar(BarWidget *bar_widget, const BarWidgetConfig *config, GuiScreen *parent);

/**
 * @brief   Update the current value of a bar widget
 * @param   bar_widget Pointer to the bar widget runtime object
 * @param   value Current bar value
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lvgl_widgets_set_bar_value(BarWidget *bar_widget, int32_t value);

/** @} */
