#pragma once

/************************************************************************************************
 * @file    lv_rle.h
 *
 * @brief   Lv Rle
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"

/* Intra-component Headers */

/**
 * @defgroup lv_rle
 * @brief    lv_rle Firmware
 * @{
 */

#ifndef LV_RLE_H
#define LV_RLE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_RLE

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

uint32_t lv_rle_decompress(const uint8_t * input,
                           uint32_t input_buff_len, uint8_t * output,
                           uint32_t output_buff_len, uint8_t blk_size);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_RLE*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_RLE_H*/

/** @} */
