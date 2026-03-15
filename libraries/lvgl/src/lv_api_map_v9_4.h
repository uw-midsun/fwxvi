#pragma once

/************************************************************************************************
 * @file    lv_api_map_v9_4.h
 *
 * @brief   Lv Api Map V9 4
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup lv_api_map_v9_4
 * @brief    lv_api_map_v9_4 Firmware
 * @{
 */

#ifndef LV_API_MAP_V9_4_H
#define LV_API_MAP_V9_4_H

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

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#define lv_tabview_rename_tab lv_tabview_set_tab_text
#define lv_wayland_timer_handler    lv_timer_handler
#define lv_wayland_display_close_f_t    lv_wayland_display_close_cb_t

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* LV_API_MAP_V9_4_H */

/** @} */
