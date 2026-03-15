#pragma once

/************************************************************************************************
 * @file    lv_lovyan_gfx.h
 *
 * @brief   Lv Lovyan Gfx
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../../display/lv_display.h"
#include "../../../indev/lv_indev.h"

/* Intra-component Headers */

/**
 * @defgroup lv_lovyan_gfx
 * @brief    lv_lovyan_gfx Firmware
 * @{
 */

#ifndef LV_LOVYAN_GFX_H
#define LV_LOVYAN_GFX_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_LOVYAN_GFX

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
lv_display_t * lv_lovyan_gfx_create(uint32_t hor_res, uint32_t ver_res, void * buf, uint32_t buf_size_bytes,
                                    bool touch);

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_LOVYAN_GFX */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_LOVYAN_GFX_H */

/** @} */
