#pragma once

/************************************************************************************************
 * @file    vg_lite_platform.h
 *
 * @brief   Vg Lite Platform
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdio.h>

/* Inter-component Headers */
#include "../../../lv_conf_internal.h"
#include "../VGLiteKernel/vg_lite_debug.h"
#include "../VGLiteKernel/vg_lite_option.h"
#include "../VGLiteKernel/vg_lite_type.h"

/* Intra-component Headers */
#include "stdint.h"
#include "stdlib.h"

/**
 * @defgroup vg_lite_platform
 * @brief    vg_lite_platform Firmware
 * @{
 */

#ifndef VG_LITE_PLATFORM_H
#define VG_LITE_PLATFORM_H

#if LV_USE_VG_LITE_DRIVER

#define VG_SYSTEM_RESERVE_COUNT 2

/* Implementation of list. ****************************************/
typedef struct list_head {
    struct list_head * next;
    struct list_head * prev;
} list_head_t;

typedef struct heap_node {
    list_head_t list;
    uint32_t offset;
    unsigned long size;
    int32_t status;
    vg_lite_vidmem_pool_t pool;
} heap_node_t;

typedef struct vg_module_parameters {

    uint32_t        register_mem_base;
    uint32_t        gpu_mem_base[VG_SYSTEM_RESERVE_COUNT];

    volatile void * contiguous_mem_base[VG_SYSTEM_RESERVE_COUNT];
    uint32_t        contiguous_mem_size[VG_SYSTEM_RESERVE_COUNT];
}
vg_module_parameters_t;

/*!
@brief Initialize the hardware mem setting.
*/
void vg_lite_init_mem(vg_module_parameters_t * param);

/*!
@brief The hardware IRQ handler.
*/
void vg_lite_IRQHandler(void);

#endif /* LV_USE_VG_LITE_DRIVER */

#endif /* VG_LITE_PLATFORM_H */
/** @} */
