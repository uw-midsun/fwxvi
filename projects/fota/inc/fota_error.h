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
typedef enum {
  FOTA_ERROR_SUCCESS,        /**< Success error code */
  FOTA_RESOURCE_EXHAUSTED,   /**< Resource exhausted error code*/
  FOTA_ERROR_INVALID_ARGS,   /**< Invalid arguments error code */
  FOTA_ERROR_INTERNAL_ERROR, /**< Internal issue error code */
  FOTA_ERROR_CRC_MISMATCH    /**< CRC mismatch error code */
} FotaError;

/** @} */
