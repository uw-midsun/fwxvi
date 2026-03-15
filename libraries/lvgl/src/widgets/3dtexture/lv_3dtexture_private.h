#pragma once

/************************************************************************************************
 * @file    lv_3dtexture_private.h
 *
 * @brief   Lv 3Dtexture Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../core/lv_obj_private.h"
#include "lv_3dtexture.h"

/* Intra-component Headers */

/**
 * @defgroup lv_3dtexture_private
 * @brief    lv_3dtexture_private Firmware
 * @{
 */

#ifndef LV_3DTEXTURE_PRIVATE_H
#define LV_3DTEXTURE_PRIVATE_H

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

/*Data of 3dtexture*/
struct _lv_3dtexture_t {
    lv_obj_t obj;
    lv_3dtexture_id_t id;
    bool h_flip;
    bool v_flip;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_3DTEXTURE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_3DTEXTURE_PRIVATE_H*/

/** @} */
