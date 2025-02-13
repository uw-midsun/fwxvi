/************************************************************************************************
 * @file   bms_carrier.c
 *
 * @brief  Source code for Battery management system
 *
 * @date   2025-01-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "bms_carrier.h"

StatusCode bms_carrier_init(BmsStorage *storage, BmsConfig *config) {
  if (storage == NULL || config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  return STATUS_CODE_OK;
}
