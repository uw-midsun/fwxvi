#pragma once

/************************************************************************************************
 * @file   fota_error.h
 *
 * @brief  Error codes for firmware over the air (FOTA) updates
 *
 * @date   2025-04-03
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup FOTA
 * @brief    Firmware Over the Air (FOTA) Update system
 * @{
 */

/**
 * @brief   FOTA Error code definitions
 */
typedef enum FotaError {
  FOTA_ERROR_SUCCESS,            /**< Success error code */
  FOTA_ERROR_RESOURCE_EXHAUSTED, /**< Resource exhausted error code*/
  FOTA_ERROR_INVALID_ARGS,       /**< Invalid arguments error code */
  FOTA_ERROR_INTERNAL_ERROR,     /**< Internal issue error code */
  FOTA_ERROR_TIMEOUT,            /**< Timeout error */

  FOTA_ERROR_CRC32_MISMATCH,         /**< CRC mismatch error code */
  FOTA_ERROR_CRC32_DATA_NOT_ALIGNED, /**< Data not 4-byte aligned in CRC32 calculation */

  FOTA_ERROR_FLASH_WRITE_FAILED, /**< */
  FOTA_ERROR_FLASH_WRITE_OUT_OF_BOUNDS,
  FOTA_ERROR_FLASH_WRITE_NOT_ALIGNED,
  FOTA_ERROR_FLASH_READ_FAILED,
  FOTA_ERROR_FLASH_ERASE,
  FOTA_ERROR_FLASH_VERIFICATION_FAILED,

  FOTA_ERROR_INVALID_PACKET,

  // DFU errors
  FOTA_ERROR_BOOTLOADER_SUCCESS,
  FOTA_ERROR_BOOTLOADER_INVALID_STATE,
  FOTA_ERROR_BOOTLOADER_UNINITIALIZED,
  FOTA_ERROR_BOOTLOADER_INVALID_DATAGRAM,
  FOTA_ERROR_BOOTLOADER_WRITE_IN_PROGRESS,
  FOTA_ERROR_BOOTLOADER_FAILURE,

  FOTA_ERROR_NO_MEMORY,
  FOTA_ERROR_NO_DATAGRAM_FOUND,

  FOTA_ERROR_JUMP_FAILED,
} FotaError;

/**
 * @brief Use to forward failures or continue on success
 */
#define fota_error_ok_or_return(code)   \
  ({                                    \
    __typeof__(code) fota_err = (code); \
    if (fota_err) return fota_err;      \
  })

/** @} */
