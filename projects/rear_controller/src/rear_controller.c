/************************************************************************************************
 * @file    rear_controller.c
 *
 * @brief   Rear controller main source file
 *
 * @date    2025-10-08
 * @author  Midnight Sun Team #24 - MSXVI (revised by hungwn2/@copilot)
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */
#include "log.h"
#include "status.h"

/* Intra-component Headers */
#include "rear_controller.h"
#include "rear_controller_state_manager.h"
#include "relays.h"

static RearControllerStorage *rear_controller_storage = NULL;

StatusCode rear_controller_init(RearControllerStorage *storage, RearControllerConfig *config) {
  if (storage == NULL || config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  rear_controller_storage = storage;
  storage->config = config;

  storage->pos_relay_closed = false;
  storage->neg_relay_closed = false;
  storage->solar_relay_closed = false;
  storage->motor_relay_closed = false;
  storage->motor_lv_enabled = false;
  storage->precharge_complete = false;
  storage->killswitch_active = false;
  storage->pcs_valid = false;
  storage->aux_valid = false;

  status_ok_or_return(relays_init(storage));
  status_ok_or_return(rear_controller_state_manager_init(storage));

  LOG_DEBUG("Rear controller initialized\r\n");
  return STATUS_CODE_OK;
}
