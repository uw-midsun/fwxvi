#pragma once

/************************************************************************************************
 * @file    lv_windows_input_private.h
 *
 * @brief   Lv Windows Input Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <windows.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup lv_windows_input_private
 * @brief    lv_windows_input_private Firmware
 * @{
 */

#ifndef LV_WINDOWS_INPUT_PRIVATE_H
#define LV_WINDOWS_INPUT_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_WINDOWS

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

bool lv_windows_pointer_device_window_message_handler(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    LRESULT * plResult);

bool lv_windows_keypad_device_window_message_handler(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    LRESULT * plResult);

bool lv_windows_encoder_device_window_message_handler(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    LRESULT * plResult);

/**********************
 *      MACROS
 **********************/

#endif // LV_USE_WINDOWS

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_WINDOWS_INPUT_PRIVATE_H*/

/** @} */
