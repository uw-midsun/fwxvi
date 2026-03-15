#pragma once

/************************************************************************************************
 * @file    lv_draw_sw_helium.h
 *
 * @brief   Lv Draw Sw Helium
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../../lv_conf_internal.h"
#include "lv_draw_sw_arm2d.h"

/* Intra-component Headers */
#include LV_DRAW_SW_HELIUM_CUSTOM_INCLUDE

/**
 * @defgroup lv_draw_sw_helium
 * @brief    lv_draw_sw_helium Firmware
 * @{
 */

#ifndef LV_DRAW_SW_HELIUM_H
#define LV_DRAW_SW_HELIUM_H

#ifdef __cplusplus
extern "C" {
#endif

/* *INDENT-OFF* */

/*********************
 *      INCLUDES
 *********************/

/* detect whether helium is available based on arm compilers' standard */
#if defined(__ARM_FEATURE_MVE) && __ARM_FEATURE_MVE

#ifdef LV_DRAW_SW_HELIUM_CUSTOM_INCLUDE
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
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/*********************
 *   POST INCLUDES
 *********************/
/* use arm-2d as the default helium acceleration */

#endif /* defined(__ARM_FEATURE_MVE) && __ARM_FEATURE_MVE */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_SW_HELIUM_H*/

/** @} */
