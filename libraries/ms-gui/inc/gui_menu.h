#pragma once

/************************************************************************************************
 * @file   gui_menu.h
 *
 * @brief  Overlay menu helpers for GUI navigation
 *
 * @date   2026-04-11
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "clut.h"
#include "display_defs.h"
#include "global_enums.h"
#include "gui_menu.h"
#include "gui_screens.h"
#include "lvgl_screens.h"
#include "status.h"

/* Intra-component Headers */

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

typedef StatusCode (*GuiMenuActionCallback)(void);

#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)

/** @brief Action Type for menu (Open new screen or do something) */
typedef enum {
  GUI_MENU_ITEM_SCREEN = 0,
  GUI_MENU_ITEM_ACTION,
} GuiMenuItemType;

/** @brief Union to store info about each menu item */
typedef struct {
  GuiMenuItemType type;     /**< Type of item (screen or action) */
  const char *action_label; /**< Label to show up on the menu screen */
  union {
    GuiScreenId screen_id;          /**< Screen ID (to distinguish between screens) */
    GuiMenuActionCallback callback; /**< Callback function for action item */
  } target;
} GuiMenuItem;

/** @brief Store menu state */
typedef struct {
  bool is_open;                                    /**< Bool to check if menu is currently open */
  uint8_t selected_index;                          /**< Index user is currently "hovering" over */
  GuiMenuActionCallback party_mode_callback;       /**< Callback for party mode */
  GuiMenuActionCallback toggle_discharge_callback; /**< Callback for toggling cell discharge */
  lv_obj_t *overlay;                               /**< Dimmed backdrop when menu is open */
  lv_obj_t *panel;                                 /**< Container that holds title + menu rows */
  lv_obj_t *title;                                 /**< Menu title */
  lv_obj_t *rows[GUI_MENU_ITEM_COUNT];             /**< LVGL Object to store Menu rows */
  lv_obj_t *row_labels[GUI_MENU_ITEM_COUNT];       /**< Labels for rows */
} GuiMenuState;

/** @brief To store pending requests for menu */
typedef struct {
  bool toggle_requested;                /**< Request flag to open/close the menu */
  bool select_requested;                /**< Request flag to activate the selected menu item */
  VehicleDriveState select_drive_state; /**< Drive state snapshot used to validate selection */
  uint8_t move_up_count;                /**< Number of queued "move up" steps */
  uint8_t move_down_count;              /**< Number of queued "move down" steps */
} GuiMenuPendingRequests;

#endif

/**
 * @brief   Initialize the overlay menu state
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_menu_init(void);

/**
 * @brief   Register the callback invoked by the Party Mode menu item
 * @param   callback Callback to invoke when Party Mode is selected
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_menu_set_party_mode_callback(GuiMenuActionCallback callback);

/**
 * @brief   Queue a menu toggle request for processing on the display task
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_menu_request_toggle(void);

/**
 * @brief   Queue an upward selection movement for processing on the display task
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_menu_request_move_up(void);

/**
 * @brief   Queue a downward selection movement for processing on the display task
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_menu_request_move_down(void);

/**
 * @brief   Queue activation of the current menu item for processing on the display task
 * @param   drive_state Current confirmed vehicle drive state used to validate menu actions
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_menu_request_select(VehicleDriveState drive_state);

/**
 * @brief   Apply queued menu requests on the display task
 * @return  STATUS_CODE_OK when all queued requests were handled
 *          STATUS_CODE_INVALID_ARGS when a queued selection is not allowed in the current drive state
 *          error otherwise
 */
StatusCode gui_menu_process_pending(void);

/**
 * @brief   Open the overlay menu on top of the current screen
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_menu_open(void);

/**
 * @brief   Close the overlay menu if it is open
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_menu_close(void);

/**
 * @brief   Toggle the overlay menu
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_menu_toggle(void);

/**
 * @brief   Move the current menu selection up
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_menu_move_up(void);

/**
 * @brief   Move the current menu selection down
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_menu_move_down(void);

/**
 * @brief   Activate the currently selected menu item
 * @param   drive_state Current confirmed vehicle drive state used to validate menu actions
 * @return  STATUS_CODE_OK on success
 *          STATUS_CODE_INVALID_ARGS when the selected item is not allowed in the current drive state
 *          error otherwise
 */
StatusCode gui_menu_select(VehicleDriveState drive_state);

/**
 * @brief   Check whether the overlay menu is currently open
 * @return  TRUE when open, FALSE otherwise
 */
bool gui_menu_is_open(void);

/** @} */
