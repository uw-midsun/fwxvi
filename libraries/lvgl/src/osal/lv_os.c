/************************************************************************************************
 * @file    lv_os.c
 *
 * @brief   Lv Os
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../core/lv_global.h"
#include "../tick/lv_tick.h"
#include "lv_os_private.h"

/* Intra-component Headers */

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/
#define lv_general_mutex LV_GLOBAL_DEFAULT()->lv_general_mutex

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_os_init(void)
{
#if LV_USE_OS != LV_OS_NONE
    lv_mutex_init(&lv_general_mutex);
#endif
}

void lv_lock(void)
{
#if LV_USE_OS != LV_OS_NONE
    lv_mutex_lock(&lv_general_mutex);
#endif
}

lv_result_t lv_lock_isr(void)
{
#if LV_USE_OS != LV_OS_NONE
    return lv_mutex_lock_isr(&lv_general_mutex);
#else
    return LV_RESULT_OK;
#endif
}

void lv_unlock(void)
{
#if LV_USE_OS != LV_OS_NONE
    lv_mutex_unlock(&lv_general_mutex);
#endif
}

#if LV_USE_OS == LV_OS_NONE
void lv_sleep_ms(uint32_t ms)
{
    lv_delay_ms(ms);
}
#endif

/**********************
 *   STATIC FUNCTIONS
 **********************/

