#pragma once

/************************************************************************************************
 * @file    lv_draw_buf_convert.h
 *
 * @brief   Lv Draw Buf Convert
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../misc/lv_color.h"
#include "../lv_draw_buf.h"

/* Intra-component Headers */

/**
 * @defgroup lv_draw_buf_convert
 * @brief    lv_draw_buf_convert Firmware
 * @{
 */

#ifndef LV_DRAW_BUF_CONVERT_H
#define LV_DRAW_BUF_CONVERT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Convert draw_buf  to premultiplied format
 * @param buf     pointer to a draw buf
 */
lv_result_t lv_draw_buf_convert_premultiply(lv_draw_buf_t * buf);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_DRAW_BUF_CONVERT_H */

/** @} */
