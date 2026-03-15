#pragma once

/************************************************************************************************
 * @file    lv_table_private.h
 *
 * @brief   Lv Table Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../core/lv_obj_private.h"
#include "lv_table.h"

/* Intra-component Headers */

/**
 * @defgroup lv_table_private
 * @brief    lv_table_private Firmware
 * @{
 */

#ifndef LV_TABLE_PRIVATE_H
#define LV_TABLE_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_TABLE != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/** Cell data */
struct _lv_table_cell_t {
    lv_table_cell_ctrl_t ctrl;
    void * user_data; /**< Custom user data */
    char txt[1];      /**< Variable length array */
};

/** Table data */
struct _lv_table_t {
    lv_obj_t obj;
    uint32_t col_cnt;
    uint32_t row_cnt;
    lv_table_cell_t ** cell_data;
    int32_t * row_h;
    int32_t * col_w;
    uint32_t col_act;
    uint32_t row_act;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_TABLE != 0 */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_TABLE_PRIVATE_H*/

/** @} */
