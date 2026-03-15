#pragma once

/************************************************************************************************
 * @file    lv_ext_data.h
 *
 * @brief   Lv Ext Data
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../lv_conf_internal.h"

/* Intra-component Headers */

/**
 * @defgroup lv_ext_data
 * @brief    lv_ext_data Firmware
 * @{
 */

#ifndef LV_EXT_DATA_H
#define LV_EXT_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
#if LV_USE_EXT_DATA
typedef struct {
    void * data;
    void (* free_cb)(void * data);
} lv_ext_data_t;
#endif

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* LV_EXT_DATA_H */

/** @} */
