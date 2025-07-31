#pragma once

/************************************************************************************************
 * @file   misc.h
 *
 * @brief  Common helper functions
 *
 * @date   2024-10-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup Miscellaneous
 * @brief    Miscellaneous libraries
 * @{
 */

#define SIZEOF_ARRAY(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define SIZEOF_FIELD(type, field) (sizeof(((type *)0)->field))
#define SWAP_UINT16(x) (uint16_t)(((uint16_t)(x) >> 8) | ((uint16_t)(x) << 8))

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

#define MIN(a, b)           \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a < _b ? _a : _b;      \
  })

#define MAX(a, b)           \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a > _b ? _a : _b;      \
  })

/** @} */
