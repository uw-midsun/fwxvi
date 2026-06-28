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
#include <string.h>

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
  STATUS_CODE_ALREADY_INITIALIZED,
  NUM_STATUS_CODES,
} StatusCode;

typedef enum { FILE_TELEMETRY = 0, FILE_CAN_COMMUNICATION, FILE_STEERING, FILE_REAR_CONTROLLER, FILE_FRONT_CONTROLLER, FILE_NUM_FILES } File;

typedef struct {
  File file;
  const char *name;
} FileStringLut;

static const FileStringLut file_string_lut[] = {
  { FILE_TELEMETRY, "telemetry.c" },
  { FILE_STEERING, "steering.c" },
  { FILE_REAR_CONTROLLER, "rear_controller.c" },
  { FILE_FRONT_CONTROLLER, "front_controller.c" },
};

/**
 * @brief Use to forward failures or continue on success.
 */
#define status_ok_or_return(code)          \
  ({                                       \
    __typeof__(code) status_expr = (code); \
    if (status_expr) return status_expr;   \
  })

#define telemetry_status_ok_or_warning(code)                       \
  ({                                                               \
    __typeof__(code) status_expr = (code);                         \
    if (status_expr) {                                             \
      for (uint8_t i = 0; i < FILE_NUM_FILES; i++) {               \
        if (strcmp(file_string_lut[i].name, __FILE__) == 0) {      \
          if (file_string_lut[i].file) {                           \
            \ set_telemetry_error_msg_status_code(status_expr);    \
            set_telemetry_error_msg_file(file_string_lut[i].file); \
          }                                                        \
        }                                                          \
      }                                                            \
    }                                                              \
  })

#define front_controller_status_ok_or_warning(code)                       \
  ({                                                                      \
    __typeof__(code) status_expr = (code);                                \
    if (status_expr) {                                                    \
      for (uint8_t i = 0; i < FILE_NUM_FILES; i++) {                      \
        if (strcmp(file_string_lut[i].name, __FILE__) == 0) {             \
          if (file_string_lut[i].file) {                                  \
            \ set_front_controller_error_msg_status_code(status_expr);    \
            set_front_controller_error_msg_file(file_string_lut[i].file); \
          }                                                               \
        }                                                                 \
      }                                                                   \
    }                                                                     \
  })

#define rear_controller_status_ok_or_warning(code)                       \
  ({                                                                     \
    __typeof__(code) status_expr = (code);                               \
    if (status_expr) {                                                   \
      for (uint8_t i = 0; i < FILE_NUM_FILES; i++) {                     \
        if (strcmp(file_string_lut[i].name, __FILE__) == 0) {            \
          if (file_string_lut[i].file) {                                 \
            set_rear_controller_error_msg_status_code(status_expr);      \
            set_rear_controller_error_msg_file(file_string_lut[i].file); \
          }                                                              \
        }                                                                \
      }                                                                  \
    }                                                                    \
  })

#define steering_status_ok_or_warning(code)                       \
  ({                                                              \
    __typeof__(code) status_expr = (code);                        \
    if (status_expr) {                                            \
      for (uint8_t i = 0; i < FILE_NUM_FILES; i++) {              \
        if (strcmp(file_string_lut[i].name, __FILE__) == 0) {     \
          if (file_string_lut[i].file) {                          \
            \ set_steering_error_msg_status_code(status_expr);    \
            set_steering_error_msg_file(file_string_lut[i].file); \
          }                                                       \
        }                                                         \
      }                                                           \
    }                                                             \
  })
/** @} */
