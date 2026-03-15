#pragma once

/************************************************************************************************
 * @file    lv_tjpgd.h
 *
 * @brief   Lv Tjpgd
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup lv_tjpgd
 * @brief    lv_tjpgd Firmware
 * @{
 */

#ifndef LV_TJPGD_H
#define LV_TJPGD_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_TJPGD

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_tjpgd_init(void);

void lv_tjpgd_deinit(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_TJPGD*/

#ifdef __cplusplus
}
#endif

#endif /* LV_TJPGD_H */

/** @} */
