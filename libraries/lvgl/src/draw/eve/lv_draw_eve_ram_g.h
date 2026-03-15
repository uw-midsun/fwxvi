#pragma once

/************************************************************************************************
 * @file    lv_draw_eve_ram_g.h
 *
 * @brief   Lv Draw Eve Ram G
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "lv_draw_eve.h"

/* Intra-component Headers */

/**
 * @defgroup lv_draw_eve_ram_g
 * @brief    lv_draw_eve_ram_g Firmware
 * @{
 */

/*  Created on: 19 nov 2023
 *      Author: juanj
 *
 *  Modified by LVGL
 */

#ifndef LV_DRAW_EVE_RAM_G_H
#define LV_DRAW_EVE_RAM_G_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if LV_USE_DRAW_EVE

/*********************
 *      DEFINES
 *********************/

#define LV_DRAW_EVE_RAMG_OUT_OF_RAMG UINT32_MAX

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

bool lv_draw_eve_ramg_get_addr(uint32_t * addr_dst, uintptr_t key,
                               uint32_t addr_size, uint32_t addr_align);

#endif/*LV_USE_DRAW_EVE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* LV_DRAW_EVE_RAM_G_H */

/** @} */
