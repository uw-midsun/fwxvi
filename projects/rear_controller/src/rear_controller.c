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
#include "relays.h" // Include the relays module header
#include "status.h"

StatusCode rear_controller_init(RearControllerStorage *storage, RearControllerConfig *config) {
  if (storage == NULL || config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  // Assign the configuration
  storage->config = config;

  // Initialize all status flags to their default values
  storage->pos_relay_closed = false;
  storage->neg_relay_closed = false;
  storage->solar_relay_closed = false;
  storage->motor_relay_closed = false;
  storage->motor_lv_enabled = false;
  storage->precharge_complete = false;
  storage->killswitch_active = false;
  storage->pcs_valid = false;
  storage->aux_valid = false;

  // Initialize the relays module, passing the main storage struct.
  status_ok_or_return(relays_init(storage));

  LOG_DEBUG("Rear controller initialized\n");
  return STATUS_CODE_OK;
}

StatusCode relay_fault(void) {
  LOG_DEBUG("Entering fault state\n");
  return relays_fault();
}

StatusCode relay_solar_close(void) {
  LOG_DEBUG("Closing solar relay\n");
  return relay_close_solar();
}

StatusCode relay_solar_open(void) {
  LOG_DEBUG("Opening solar relay\n");
  return relay_open_solar();
}

StatusCode relay_motor_close(void) {
  LOG_DEBUG("Closing motor relay\n");
  return rear_close_motor();
}

StatusCode relay_motor_open(void) {
  LOG_DEBUG("Opening motor relay\n");
  return rear_open_motor();
}