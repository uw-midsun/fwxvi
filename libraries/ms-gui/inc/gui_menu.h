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
#include "global_enums.h"
#include "status.h"

/* Intra-component Headers */

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

typedef StatusCode (*GuiMenuActionCallback)(void);

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
