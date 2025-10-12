/************************************************************************************************
 * @file    rear_controller.c
 *
 * @brief   Rear controller main source file
 *
 * @date    2025-10-08
 * @author  Midnight Sun Team #24 - MSXVI (revised by hungwn2/@copilot)
 ************************************************************************************************/

#include "rear_controller.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "log.h"
#include "relays.h" 
#include "status.h"


static RearControllerStorage *s_rear_controller_storage=NULL;
StatusCode rear_controller_init(RearControllerStorage *storage, RearControllerConfig *config) {
  if (storage == NULL || config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_rear_controller_storage = storage;
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

  LOG_DEBUG("Rear controller initialized\n");
  return STATUS_CODE_OK;
}

StatusCode rear_fault(void) {
  LOG_DEBUG("Entering fault state\n");
  return relays_fault();
}

StatusCode rear_solar_close(void) {
  LOG_DEBUG("Closing solar relay\n");
  return relays_close_solar();
}

StatusCode rear_solar_open(void) {
  LOG_DEBUG("Opening solar relay\n");
  return relays_open_solar();
}

StatusCode rear_motor_close(void) {
  LOG_DEBUG("Closing motor relay\n");
  return relays_close_motor();
}

StatusCode rear_motor_open(void) {
  LOG_DEBUG("Opening motor relay\n");
  return relays_open_motor();
}

StatusCode rear_controller_deinit(void) {
    s_rear_controller_storage = NULL;
    relays_deinit();
    return STATUS_CODE_OK;
}