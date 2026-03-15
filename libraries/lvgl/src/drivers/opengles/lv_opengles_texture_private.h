#pragma once

/************************************************************************************************
 * @file    lv_opengles_texture_private.h
 *
 * @brief   Lv Opengles Texture Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"
#include "lv_opengles_texture.h"

/* Intra-component Headers */

/**
 * @defgroup lv_opengles_texture_private
 * @brief    lv_opengles_texture_private Firmware
 * @{
 */

#ifndef LV_OPENGLES_TEXTURE_PRIVATE_H
#define LV_OPENGLES_TEXTURE_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_OPENGLES

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    unsigned int texture_id;
#if !LV_USE_DRAW_OPENGLES
    uint8_t * fb1;
#endif /*!LV_USE_DRAW_OPENGLES*/
    bool is_texture_owner;
} lv_opengles_texture_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

lv_result_t lv_opengles_texture_reshape(lv_opengles_texture_t * texture, lv_display_t * display,
                                        int32_t width, int32_t height);
void lv_opengles_texture_deinit(lv_opengles_texture_t * texture);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_OPENGLES*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* LV_OPENGLES_TEXTURE_PRIVATE_H */

/** @} */
