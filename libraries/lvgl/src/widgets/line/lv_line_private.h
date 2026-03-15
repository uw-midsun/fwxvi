#pragma once

/************************************************************************************************
 * @file    lv_line_private.h
 *
 * @brief   Lv Line Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../core/lv_obj_private.h"
#include "lv_line.h"

/* Intra-component Headers */

/**
 * @defgroup lv_line_private
 * @brief    lv_line_private Firmware
 * @{
 */

#ifndef LV_LINE_PRIVATE_H
#define LV_LINE_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_LINE != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/** Data of line */
struct _lv_line_t {
    lv_obj_t obj;
    union {
        const lv_point_precise_t * constant;
        lv_point_precise_t * mut;
    } point_array;                      /**< Pointer to an array with the points of the line*/
    uint32_t point_num;                 /**< Number of points in 'point_array'*/
    uint32_t y_inv : 1;                  /**< 1: y == 0 will be on the bottom*/
    uint32_t point_array_is_mutable : 1; /**< whether the point array is const or mutable*/
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_LINE != 0 */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_LINE_PRIVATE_H*/

/** @} */
