#pragma once

/************************************************************************************************
 * @file    lv_tft_espi.h
 *
 * @brief   Lv Tft Espi
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../../display/lv_display.h"

/* Intra-component Headers */

/**
 * @defgroup lv_tft_espi
 * @brief    lv_tft_espi Firmware
 * @{
 */

#ifndef LV_TFT_ESPI_H
#define LV_TFT_ESPI_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_TFT_ESPI

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
lv_display_t * lv_tft_espi_create(uint32_t hor_res, uint32_t ver_res, void * buf, uint32_t buf_size_bytes);

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_TFT_ESPI */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_TFT_ESPI_H */

/** @} */
