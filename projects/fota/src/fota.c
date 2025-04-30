/************************************************************************************************
 * @file   fota.c
 *
 * @brief  Source file for fota
 *
 * @date   2025-03-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_encryption.h"
#include "fota_error.h"

FotaError fota_init() {
  fota_encryption_init();

  return FOTA_ERROR_SUCCESS;
}
