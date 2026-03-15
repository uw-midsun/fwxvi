#pragma once

/************************************************************************************************
 * @file    lv_nuttx_lcd.h
 *
 * @brief   Lv Nuttx Lcd
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../display/lv_display.h"

/* Intra-component Headers */

/**
 * @defgroup lv_nuttx_lcd
 * @brief    lv_nuttx_lcd Firmware
 * @{
 */

#ifndef LV_NUTTX_LCD_H
#define LV_NUTTX_LCD_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_NUTTX

#if LV_USE_NUTTX_LCD

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

lv_display_t * lv_nuttx_lcd_create(const char * dev_path);

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_NUTTX_LCD */

#endif /* LV_USE_NUTTX*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_NUTTX_LCD_H */

/** @} */
