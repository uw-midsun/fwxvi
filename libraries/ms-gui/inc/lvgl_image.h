#pragma once

/************************************************************************************************
 * @file   lvgl_image.h
 *
 * @brief  LVGL-specific image widget wrapper
 *
 * @date   2026-05-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
#include "lvgl.h"
#endif
#include "status.h"

/* Intra-component Headers */
#include "clut.h"
#include "lvgl_screens.h"
#include "lvgl_widgets.h"

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
typedef lv_obj_t LvglImageObject;
#else
typedef void LvglImageObject;
#endif

/** @brief   Runtime handle for an image widget */
typedef struct {
  LvglImageObject* image; /**< LVGL image object */
} LvglImage;

/** @brief   Configuration used when creating an image widget */
typedef struct {
  WidgetSize size;             /**< Bounding size of the image widget, or content-sized when zero */
  WidgetPosition position;     /**< Placement of the image widget on screen */
  const void* source;          /**< LVGL image source, such as an lv_image_dsc_t pointer or symbol */
  bool recolor_enabled;        /**< Whetrher to recolor the image source */
  GuiColorId recolor_color_id; /**< Color used for image recoloring when enabled */
} LvglImageConfig;

/**
 * @brief   Create and initialize an image widget
 * @param   image Pointer to the runtime image object to initialize
 * @param   config Pointer to the widget configuration
 * @param   parent Parent LVGL object that will own the image
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lvgl_image_create(LvglImage* image, const LvglImageConfig* config, GuiScreen* parent);

/**
 * @brief   Update the source displayed by an image widget
 * @param   image Pointer to the runtime image object
 * @param   source LVGL image source, such as an lv_imge_dsc_t pointer or symbol
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lvgl_image_set_source(LvglImage* image, const void* source);

/**
 * @brief   Apply a semantic GUI color as an image recolor
 * @param   image Pointer to the runtime image object
 * @param   color_id Semantic color identifier
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lvgl_image_set_recolor(LvglImage* image, GuiColorId color_id);

/**
 * @brief   Disable image recoloring
 * @param   image Pointer to the runtime image object
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lvgl_image_clear_recolor(LvglImage* image);

/** @} */
