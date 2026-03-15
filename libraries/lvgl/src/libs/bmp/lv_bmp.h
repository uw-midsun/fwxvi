#pragma once

/************************************************************************************************
 * @file    lv_bmp.h
 *
 * @brief   Lv Bmp
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"

/* Intra-component Headers */

/**
 * @defgroup lv_bmp
 * @brief    lv_bmp Firmware
 * @{
 */

#ifndef LV_BMP_H
#define LV_BMP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if LV_USE_BMP

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_bmp_init(void);
void lv_bmp_deinit(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_BMP*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_BMP_H*/

/** @} */
