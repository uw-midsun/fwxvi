#pragma once

/************************************************************************************************
 * @file    lv_rb_private.h
 *
 * @brief   Lv Rb Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "lv_rb.h"

/* Intra-component Headers */

/**
 * @defgroup lv_rb_private
 * @brief    lv_rb_private Firmware
 * @{
 */

#ifndef LV_RB_PRIVATE_H
#define LV_RB_PRIVATE_H

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

struct _lv_rb_node_t {
    struct _lv_rb_node_t * parent;
    struct _lv_rb_node_t * left;
    struct _lv_rb_node_t * right;
    lv_rb_color_t color;
    void * data;
};

struct _lv_rb_t {
    lv_rb_node_t * root;
    lv_rb_compare_t compare;
    size_t size;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_RB_PRIVATE_H*/

/** @} */
