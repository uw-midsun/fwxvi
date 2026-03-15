#pragma once

/************************************************************************************************
 * @file    lv_vg_lite_decoder.h
 *
 * @brief   Lv Vg Lite Decoder
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"

/* Intra-component Headers */

/**
 * @defgroup lv_vg_lite_decoder
 * @brief    lv_vg_lite_decoder Firmware
 * @{
 */

#ifndef LV_VG_LITE_DECODER_H
#define LV_VG_LITE_DECODER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_DRAW_VG_LITE

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_vg_lite_decoder_init(void);

void lv_vg_lite_decoder_deinit(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DRAW_VG_LITE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_VG_LITE_DECODER_H*/

/** @} */
