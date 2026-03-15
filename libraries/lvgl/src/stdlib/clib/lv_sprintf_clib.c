/************************************************************************************************
 * @file    lv_sprintf_clib.c
 *
 * @brief   Lv Sprintf Clib
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdarg.h>
#include <stdio.h>

/* Inter-component Headers */
#include "../../lv_conf_internal.h"
#include "../lv_sprintf.h"

/* Intra-component Headers */

/*********************
 *      INCLUDES
 *********************/
#if LV_USE_STDLIB_SPRINTF == LV_STDLIB_CLIB

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

int lv_snprintf(char * buffer, size_t count, const char * format, ...)
{
    va_list va;
    va_start(va, format);
    const int ret = vsnprintf(buffer, count, format, va);
    va_end(va);
    return ret;
}

int lv_vsnprintf(char * buffer, size_t count, const char * format, va_list va)
{
    return vsnprintf(buffer, count, format, va);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
