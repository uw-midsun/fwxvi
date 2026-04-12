#pragma once

/************************************************************************************************
 * @file   gui_screens.h
 *
 * @brief  High-level screen registry and navigation
 *
 * @date   2026-04-11
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "lvgl_screens.h"

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

typedef enum {
  GUI_SCREEN_DRIVE = 0,
  GUI_SCREEN_TRIP_INFO,
  GUI_SCREEN_PACK_VOLTAGE,
  GUI_SCREEN_PEDAL_CALIB,
  NUM_GUI_SCREENS,
} GuiScreenId;

typedef StatusCode (*GuiScreenCreateFn)(GuiScreen *screen);

typedef struct {
  GuiScreenId id;
  const char *name;
  GuiScreenCreateFn create;
} GuiScreenDescriptor;

/**
 * @brief   Initialize the high-level screen registry and show the drive screen
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_screens_init(void);

/**
 * @brief   Register or override a screen descriptor
 * @param   descriptor Screen descriptor to register
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_screens_register(const GuiScreenDescriptor *descriptor);

/**
 * @brief   Show a registered screen
 * @param   screen_id Screen to load
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_screens_show(GuiScreenId screen_id);

/**
 * @brief   Get the currently active high-level screen id
 * @return  Currently active screen id
 */
GuiScreenId gui_screens_get_current(void);

/**
 * @brief   Get the root LVGL object for a registered screen
 * @param   screen_id Screen to query
 * @return  Screen root pointer, or NULL when unavailable
 */
GuiScreen *gui_screens_get_root(GuiScreenId screen_id);

/**
 * @brief   Get the display name for a registered screen
 * @param   screen_id Screen to query
 * @return  Screen name, or NULL when unavailable
 */
const char *gui_screens_get_name(GuiScreenId screen_id);

/** @} */
