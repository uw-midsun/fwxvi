#pragma once

/************************************************************************************************
 * @file    lv_sdl_keyboard.h
 *
 * @brief   Lv Sdl Keyboard
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "lv_sdl_window.h"

/* Intra-component Headers */

/**
 * @defgroup lv_sdl_keyboard
 * @brief    lv_sdl_keyboard Firmware
 * @{
 */

#ifndef LV_SDL_KEYBOARD_H
#define LV_SDL_KEYBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if LV_USE_SDL

/*********************
 *      DEFINES
 *********************/
#ifndef KEYBOARD_BUFFER_SIZE
#define KEYBOARD_BUFFER_SIZE 32
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

lv_indev_t * lv_sdl_keyboard_create(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_SDL*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_SDL_KEYBOARD_H */

/** @} */
