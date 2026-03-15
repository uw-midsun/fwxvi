#pragma once

/************************************************************************************************
 * @file    lv_pxp_osa.h
 *
 * @brief   Lv Pxp Osa
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../../lv_conf_internal.h"
#include "lv_pxp_cfg.h"

/* Intra-component Headers */

/**
 * @defgroup lv_pxp_osa
 * @brief    lv_pxp_osa Firmware
 * @{
 */

/**
 * Copyright 2020, 2022-2023 NXP
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LV_PXP_OSA_H
#define LV_PXP_OSA_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_PXP
#if LV_USE_DRAW_PXP || LV_USE_ROTATE_PXP

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
 * PXP device interrupt handler. Used to check PXP task completion status.
 */
void PXP_IRQHandler(void);

/**
 * Get the PXP default configuration.
 */
pxp_cfg_t * pxp_get_default_cfg(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DRAW_PXP || LV_USE_ROTATE_PXP*/
#endif /*LV_USE_PXP*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PXP_OSA_H*/

/** @} */
