#pragma once

/************************************************************************************************
 * @file    lv_sprintf.h
 *
 * @brief   Lv Sprintf
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../misc/lv_types.h"

/* Intra-component Headers */
        #include LV_INTTYPES_INCLUDE

/**
 * @defgroup lv_sprintf
 * @brief    lv_sprintf Firmware
 * @{
 */

#ifndef LV_SPRINTF_H
#define LV_SPRINTF_H

#if defined(__has_include)
    #if __has_include(LV_INTTYPES_INCLUDE)
        /* platform-specific printf format for int32_t, usually "d" or "ld" */
        #define LV_PRId32 PRId32
        #define LV_PRIu32 PRIu32
        #define LV_PRIx32 PRIx32
        #define LV_PRIX32 PRIX32

        #define LV_PRId64 PRId64
        #define LV_PRIu64 PRIu64
        #define LV_PRIx64 PRIx64
        #define LV_PRIX64 PRIX64
    #else
        #define LV_PRId32 "d"
        #define LV_PRIu32 "u"
        #define LV_PRIx32 "x"
        #define LV_PRIX32 "X"

        #define LV_PRId64 "lld"
        #define LV_PRIu64 "llu"
        #define LV_PRIx64 "llx"
        #define LV_PRIX64 "llX"
    #endif
#else
    /* hope this is correct for ports without __has_include or without inttypes.h */
    #define LV_PRId32 "d"
    #define LV_PRIu32 "u"
    #define LV_PRIx32 "x"
    #define LV_PRIX32 "X"

    #define LV_PRId64 "lld"
    #define LV_PRIu64 "llu"
    #define LV_PRIx64 "llx"
    #define LV_PRIX64 "llX"
#endif

#ifdef __cplusplus
extern "C" {
#endif

int lv_snprintf(char * buffer, size_t count, const char * format, ...);

int lv_vsnprintf(char * buffer, size_t count, const char * format, va_list va);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  /* LV_SPRINTF_H */

/** @} */
