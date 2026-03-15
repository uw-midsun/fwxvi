/************************************************************************************************
 * @file    lv_test_helpers.c
 *
 * @brief   Lv Test Helpers
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lvgl.h"
#include "lv_test_helpers.h"

/* Intra-component Headers */

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_TEST

/*********************
 *      DEFINES
 *********************/

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

void lv_test_wait(uint32_t ms)
{
    while(ms) {
        lv_tick_inc(1);
        lv_timer_handler();
        ms--;
    }
    lv_refr_now(NULL);
}

void lv_test_fast_forward(uint32_t ms)
{
    lv_tick_inc(ms);
    lv_timer_handler();
    lv_refr_now(NULL);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*LV_USE_TEST*/
