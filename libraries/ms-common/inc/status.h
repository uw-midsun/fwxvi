#pragma once

/************************************************************************************************
 * @file   status.h
 *
 * @brief  Status Library for more verbose error handling
 *
 * @date   2024-10-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup Status_Code
 * @brief    Global error library
 * @{
 */

/**
 * @brief StatusCodes for various errors.
 */
typedef enum {
  STATUS_CODE_OK = 0,
  STATUS_CODE_UNKNOWN,
  STATUS_CODE_INVALID_ARGS,
  STATUS_CODE_RESOURCE_EXHAUSTED,
  STATUS_CODE_UNREACHABLE,
  STATUS_CODE_TIMEOUT,
  STATUS_CODE_EMPTY,
  STATUS_CODE_OUT_OF_RANGE,
  STATUS_CODE_UNIMPLEMENTED,
  STATUS_CODE_UNINITIALIZED,
  STATUS_CODE_INTERNAL_ERROR,
  STATUS_CODE_INCOMPLETE,
  NUM_STATUS_CODES,
} StatusCode;

/**
 * @brief Use to forward failures or continue on success.
 */
#define status_ok_or_return(code)          \
  ({                                       \
    __typeof__(code) status_expr = (code); \
    if (status_expr) return status_expr;   \
  })

/** @} */
