#pragma once

/************************************************************************************************
 * @file    lv_text_ap.h
 *
 * @brief   Lv Text Ap
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../draw/lv_draw.h"
#include "lv_text.h"
#include "lv_types.h"

/* Intra-component Headers */

/**
 * @defgroup lv_text_ap
 * @brief    lv_text_ap Firmware
 * @{
 */

#ifndef LV_TEXT_AP_H
#define LV_TEXT_AP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_ARABIC_PERSIAN_CHARS == 1

/*********************
 *      DEFINES
 *********************/

#define LV_UNDEF_ARABIC_PERSIAN_CHARS     (UINT32_MAX)
#define LV_AP_ALPHABET_BASE_CODE          0x0622
#define LV_AP_END_CHARS_LIST              {0,0,0,0,0,{0,0}}
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
uint32_t lv_text_ap_calc_bytes_count(const char * txt);
void lv_text_ap_proc(const char * txt, char * txt_out);

/**********************
 *      MACROS
 **********************/

#endif // LV_USE_ARABIC_PERSIAN_CHARS

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_TEXT_AP_H*/

/** @} */
