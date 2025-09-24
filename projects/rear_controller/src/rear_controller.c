/************************************************************************************************
 * @file    rear_controller.c
 *
 * @brief   Rear Controller
 *
 * @date    2025-09-17
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "gpio.h"
#include "mcu.h"
#include "flash.h"

/* Intra-component Headers */
#include "rear_controller.h"
#include "rear_controller_hw_defs.h"

static RearControllerStorage *rear_controller_storage;

StatusCode rear_controller_init(RearControllerStorage *storage, RearControllerConfig *config) {
  if (storage == NULL || config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  rear_controller_storage = storage;
  rear_controller_storage->config = config;

  flash_init();

  return STATUS_CODE_OK;
}
