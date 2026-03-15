/************************************************************************************************
 * @file    vg_lite_os.c
 *
 * @brief   Vg Lite Os
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../../lv_conf_internal.h"
#include "../../../lvgl.h"

/* Intra-component Headers */
#if LV_USE_VG_LITE_DRIVER

void * vg_lite_os_malloc(uint32_t size)
{
    return lv_malloc(size);
}

void vg_lite_os_free(void * memory)
{
    lv_free(memory);
}

#endif
