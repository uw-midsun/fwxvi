#pragma once

/************************************************************************************************
 * @file    lv_sdl_mouse.h
 *
 * @brief   Lv Sdl Mouse
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "lv_sdl_window.h"

/* Intra-component Headers */

/**
 * @defgroup lv_sdl_mouse
 * @brief    lv_sdl_mouse Firmware
 * @{
 */

#ifndef LV_SDL_MOUSE_H
#define LV_SDL_MOUSE_H

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

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

lv_indev_t * lv_sdl_mouse_create(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_SDL*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_SDL_MOUSE_H */

/** @} */
