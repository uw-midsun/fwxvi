#pragma once

/************************************************************************************************
 * @file    lv_rb.h
 *
 * @brief   Lv Rb
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "lv_assert.h"
#include "lv_types.h"
#include "lv_types.h"

/* Intra-component Headers */

/**
 * @defgroup lv_rb
 * @brief    lv_rb Firmware
 * @{
 */

#ifndef LV_RB_H
#define LV_RB_H

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

typedef enum {
    LV_RB_COLOR_RED,
    LV_RB_COLOR_BLACK
} lv_rb_color_t;

typedef int32_t lv_rb_compare_res_t;

typedef lv_rb_compare_res_t (*lv_rb_compare_t)(const void * a, const void * b);

/**********************
 * GLOBAL PROTOTYPES
 **********************/

bool lv_rb_init(lv_rb_t * tree, lv_rb_compare_t compare, size_t node_size);
lv_rb_node_t * lv_rb_insert(lv_rb_t * tree, void * key);
lv_rb_node_t * lv_rb_find(lv_rb_t * tree, const void * key);
void * lv_rb_remove_node(lv_rb_t * tree, lv_rb_node_t * node);
void * lv_rb_remove(lv_rb_t * tree, const void * key);
bool lv_rb_drop_node(lv_rb_t * tree, lv_rb_node_t * node);
bool lv_rb_drop(lv_rb_t * tree, const void * key);
lv_rb_node_t * lv_rb_minimum(lv_rb_t * node);
lv_rb_node_t * lv_rb_maximum(lv_rb_t * node);
lv_rb_node_t * lv_rb_minimum_from(lv_rb_node_t * node);
lv_rb_node_t * lv_rb_maximum_from(lv_rb_node_t * node);
void lv_rb_destroy(lv_rb_t * tree);

/*************************
 *    GLOBAL VARIABLES
 *************************/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_RB_H*/

/** @} */
