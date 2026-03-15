#pragma once

/************************************************************************************************
 * @file    lv_translation_private.h
 *
 * @brief   Lv Translation Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"
#include "../../misc/lv_array.h"

/* Intra-component Headers */
#include LV_STDINT_INCLUDE

/**
 * @defgroup lv_translation_private
 * @brief    lv_translation_private Firmware
 * @{
 */

#ifndef LV_TRANSLATION_PRIVATE_H
#define LV_TRANSLATION_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_TRANSLATION

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

struct _lv_translation_tag_dsc_t {
    const char * tag;
    const char ** translations; /**< Translations for each language*/
};

struct _lv_translation_pack_t {
    const char ** languages;
    uint32_t language_cnt;
    uint32_t is_static; /*In the union translations_p is used*/
    const char ** tag_p;
    const char ** translation_p; /*E.g. {{"a", "b"}, {"c", "d"}}*/
    lv_array_t translation_array;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_TRANSLATION*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* LV_TRANSLATION_PRIVATE_H */

/** @} */
