#pragma once

/************************************************************************************************
 * @file    lv_3dtexture.h
 *
 * @brief   Lv 3Dtexture
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../core/lv_obj.h"
#include "../../lv_conf_internal.h"

/* Intra-component Headers */

/**
 * @defgroup lv_3dtexture
 * @brief    lv_3dtexture Firmware
 * @{
 */

#ifndef LV_3DTEXTURE_H
#define LV_3DTEXTURE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_3DTEXTURE

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_3dtexture_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a 3dtexture object
 * @param parent    pointer to an object, it will be the parent of the new 3dtexture
 * @return          pointer to the created 3dtexture
 */
lv_obj_t * lv_3dtexture_create(lv_obj_t * parent);

/**
 * Set the source texture of the widget.
 * The object size should be manually set to match.
 * @param obj    the 3dtexture widget
 * @param id     the texture handle from the 3D graphics backend.
 *               I.e., an `unsigned int` texture for OpenGL.
 */
void lv_3dtexture_set_src(lv_obj_t * obj, lv_3dtexture_id_t id);

/**
 * Set the flipping behavior of the widget.
 * @param obj      the 3dtexture widget
 * @param h_flip   true to flip horizontally.
 * @param v_flip   true to flip vertically.
 */
void lv_3dtexture_set_flip(lv_obj_t * obj, bool h_flip, bool v_flip);

/*======================
 * Add/remove functions
 *=====================*/

/*=====================
 * Setter functions
 *====================*/

/*=====================
 * Getter functions
 *====================*/

/*=====================
 * Other functions
 *====================*/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_3DTEXTURE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_3DTEXTURE_H*/

/** @} */
