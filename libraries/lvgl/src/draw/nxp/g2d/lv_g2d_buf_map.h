#pragma once

/************************************************************************************************
 * @file    lv_g2d_buf_map.h
 *
 * @brief   Lv G2D Buf Map
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#include "../../../lv_conf_internal.h"
#include "../../../misc/lv_array.h"

/* Intra-component Headers */

/**
 * @defgroup lv_g2d_buf_map
 * @brief    lv_g2d_buf_map Firmware
 * @{
 */

/**
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LV_G2D_BUF_MAP_H
#define LV_G2D_BUF_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_G2D
#if LV_USE_DRAW_G2D || LV_USE_ROTATE_G2D

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/* Map item definition. */
typedef struct lv_map_item {
    /* Virtual address buffer. */
    void * key;
    struct g2d_buf * value;
} lv_map_item_t;

/*Buf map definition. */
typedef struct lv_buf_map {
    lv_map_item_t ** items;
    lv_array_t ** overflow_list;

    int size;
    int count;
} lv_buf_map_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void g2d_create_buf_map(void);

void g2d_free_buf_map(void);

void g2d_insert_buf_map(void * key, struct g2d_buf * value);

struct g2d_buf * g2d_search_buf_map(void * key);

void g2d_free_item(void * key);

void g2d_print_table(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DRAW_G2D || LV_USE_ROTATE_G2D*/
#endif /*LV_USE_G2D*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* LV_G2D_BUF_MAP_H */

/** @} */
