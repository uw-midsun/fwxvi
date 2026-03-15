#pragma once

/************************************************************************************************
 * @file    lv_fs_private.h
 *
 * @brief   Lv Fs Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "lv_fs.h"

/* Intra-component Headers */

/**
 * @defgroup lv_fs_private
 * @brief    lv_fs_private Firmware
 * @{
 */

#ifndef LV_FS_PRIVATE_H
#define LV_FS_PRIVATE_H

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

struct _lv_fs_file_cache_t {
    uint32_t start;
    uint32_t end;
    uint32_t file_position;
    void * buffer;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the File system interface
 */
void lv_fs_init(void);

/**
 * Deinitialize the File system interface
 */
void lv_fs_deinit(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_FS_PRIVATE_H*/

/** @} */
