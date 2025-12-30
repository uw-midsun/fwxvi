/************************************************************************************************
 * @file    cruise_control.c
 *
 * @brief   Cruise Control
 *
 * @date    2025-11-12
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "log.h"

/* Intra-component Headers */
#include "button.h"
#include "button_manager.h"
#include "steering.h"

static SteeringStorage *steering_storage;

/* Hold state */
static uint8_t hold_ticks = 0;
static int8_t hold_direction = 0; /* +1 = up, -1 = down */

static uint8_t s_step_from_hold(uint8_t ticks) {
  if (ticks <= 1) {
    /* For hold time of 0s - 1s we increase speed by 1km/h */
    return 1;
  } else if (ticks <= 2) {
    /* For hold time of 1s - 2s we increase speed by 2km/h */
    return 2;
  } else if (ticks <= 4) {
    /* For hold time of 2s - 4s we increase speed by 5km/h */
    return 5;
  } else {
    /* For hold time of 4s+ we increase speed by 10km/h */
    return 10;
  }
}

StatusCode cruise_control_init(SteeringStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  steering_storage = storage;
  hold_ticks = 0;
  hold_direction = 0;

  return STATUS_CODE_OK;
}

StatusCode cruise_control_down_handler() {
  if (steering_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (steering_storage->cruise_control_enabled && steering_storage->cruise_control_target_speed_kmh > steering_storage->config->cruise_min_speed_kmh) {
    steering_storage->cruise_control_target_speed_kmh--;
    return STATUS_CODE_OK;
  }

  return STATUS_CODE_RESOURCE_EXHAUSTED;
}

StatusCode cruise_control_up_handler() {
  if (steering_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (steering_storage->cruise_control_enabled && steering_storage->cruise_control_target_speed_kmh < steering_storage->config->cruise_max_speed_kmh) {
    steering_storage->cruise_control_target_speed_kmh++;
    return STATUS_CODE_OK;
  }

  return STATUS_CODE_RESOURCE_EXHAUSTED;
}

StatusCode cruise_control_run() {
  if (steering_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  LOG_DEBUG("Cruise control target speed: %u\r\n", steering_storage->cruise_control_target_speed_kmh);

  ButtonState up = steering_storage->button_manager->buttons[STEERING_BUTTON_CRUISE_CONTROL_UP].state;
  ButtonState down = steering_storage->button_manager->buttons[STEERING_BUTTON_CRUISE_CONTROL_DOWN].state;

  /* Enable / Disable */
  if (up == BUTTON_PRESSED && down == BUTTON_PRESSED) {
    steering_storage->cruise_control_enabled = !steering_storage->cruise_control_enabled;
    hold_ticks = 0;
    hold_direction = 0;
    LOG_DEBUG("Cruise control %s\r\n", steering_storage->cruise_control_enabled ? "enabled" : "disabled");
    return STATUS_CODE_OK;
  }

  if (!steering_storage->cruise_control_enabled) {
    hold_ticks = 0;
    hold_direction = 0;
    return STATUS_CODE_OK;
  }

  /* Determine direction */
  int8_t direction = (up == BUTTON_PRESSED) - (down == BUTTON_PRESSED);

  /* Nothing is pressed */
  if (direction == 0) {
    hold_ticks = 0;
    hold_direction = 0;
    return STATUS_CODE_OK;
  }

  if (direction != hold_direction) {
    hold_ticks = 0;
    hold_direction = direction;
  }

  hold_ticks++;

  uint8_t step = s_step_from_hold(hold_ticks);
  while (step--) {
    if (direction > 0) {
      cruise_control_up_handler();
    } else {
      cruise_control_down_handler();
    }
  }

  return STATUS_CODE_OK;
}
