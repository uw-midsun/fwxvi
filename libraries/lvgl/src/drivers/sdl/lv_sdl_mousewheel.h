#pragma once

/************************************************************************************************
 * @file    lv_sdl_mousewheel.h
 *
 * @brief   Lv Sdl Mousewheel
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "lv_sdl_window.h"

/* Intra-component Headers */

/**
 * @defgroup lv_sdl_mousewheel
 * @brief    lv_sdl_mousewheel Firmware
 * @{
 */

#ifndef LV_SDL_MOUSEWHEEL_H
#define LV_SDL_MOUSEWHEEL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if LV_USE_SDL && LV_SDL_MOUSEWHEEL_MODE == LV_SDL_MOUSEWHEEL_MODE_ENCODER

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

lv_indev_t * lv_sdl_mousewheel_create(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_SDL*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_SDL_MOUSEWHEEL_H */

/** @} */
