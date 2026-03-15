#pragma once

/************************************************************************************************
 * @file    lv_blend_neon.h
 *
 * @brief   Lv Blend Neon
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../../../lv_conf_internal.h"
#include "lv_draw_sw_blend_neon_to_rgb565.h"
#include "lv_draw_sw_blend_neon_to_rgb888.h"

/* Intra-component Headers */
#include LV_DRAW_SW_NEON_CUSTOM_INCLUDE

/**
 * @defgroup lv_blend_neon
 * @brief    lv_blend_neon Firmware
 * @{
 */

#ifndef LV_BLEND_NEON_H
#define LV_BLEND_NEON_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_DRAW_SW_ASM == LV_DRAW_SW_ASM_NEON

#ifdef LV_DRAW_SW_NEON_CUSTOM_INCLUDE
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* #if LV_USE_DRAW_SW_ASM == LV_DRAW_SW_ASM_NEON */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_BLEND_NEON_H*/

/** @} */
