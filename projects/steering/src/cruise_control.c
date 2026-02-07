/************************************************************************************************
 * @file    cruise_control.c
 *
 * @brief   Cruise Control button handler class
 *
 * @date    2025-11-12
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <math.h>

/* Inter-component Headers */
#include "global_enums.h"
#include "log.h"

/* Intra-component Headers */
#include "button.h"
#include "button_manager.h"
#include "steering.h"
#include "steering_getters.h"
#include "steering_setters.h"

#define CC_DEBUG 1U

static SteeringStorage *steering_storage;

/* Hold state */
static uint8_t hold_ticks = 0;
static int8_t hold_direction = 0; /* +1 = up, -1 = down */

/* CC up/down button state */
static bool up_button_pressed;
static bool down_button_pressed;
static bool cruise_control_enabled_released;

static uint8_t s_step_from_hold(uint8_t ticks) {
  if (ticks == 1) {
    /* For initial press, we increase speed by 1km/h */
    return 1;
  } else if (ticks <= 5) {
    /* After increasing by 1, we should wait 0.5s before ramping up speed */
    return 0;
  } else if (ticks <= 10) {
    /* Between 0.5 - 1.5s, we increase speed by 1km/h*/
    return 1;
  } else if (ticks <= 20) {
    /* For hold time of 1s - 2s we increase speed by 5km/h */
    return 5;
  } else {
    /* For hold time of 2s+ we increase speed by 10km/h */
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
  up_button_pressed = false;
  down_button_pressed = false;
  cruise_control_enabled_released = true;

  // CC target speed should be bouned between min and max speed
  steering_storage->cruise_control_target_speed_kmh =
      fminf(steering_storage->config->cruise_max_speed_kmh, fmaxf(steering_storage->config->cruise_min_speed_kmh, steering_storage->cruise_control_target_speed_kmh));

  return STATUS_CODE_OK;
}

StatusCode cruise_control_down_handler() {
  if (steering_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (steering_storage->cruise_control_enabled && steering_storage->cruise_control_target_speed_kmh > steering_storage->config->cruise_min_speed_kmh) {
    steering_storage->cruise_control_target_speed_kmh--;
  }

  return STATUS_CODE_OK;
}

StatusCode cruise_control_up_handler() {
  if (steering_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (steering_storage->cruise_control_enabled && steering_storage->cruise_control_target_speed_kmh < steering_storage->config->cruise_max_speed_kmh) {
    steering_storage->cruise_control_target_speed_kmh++;
  }

  return STATUS_CODE_OK;
}

StatusCode cruise_control_run_medium_cycle() {
  if (steering_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  // #if (CC_DEBUG == 1)
  //   if (steering_storage->cruise_control_enabled == true) {
  //     LOG_DEBUG("Cruise control target speed: %u\r\n", steering_storage->cruise_control_target_speed_kmh);
  //   } else {
  //     LOG_DEBUG("Cruise control disabled\r\n");
  //   }
  // #endif

  // Cruise control should only work when we are in VehicleDriveState VEHICLE_DRIVE_STATE_DRIVE
  VehicleDriveState drive_state_from_front = (VehicleDriveState)get_pedal_data_drive_state();

  if (drive_state_from_front == VEHICLE_DRIVE_STATE_BRAKE && steering_storage->cruise_control_enabled) {
    LOG_DEBUG("BRAKE from front\r\n");
    steering_storage->cruise_control_enabled = false;
    set_steering_buttons_cruise_control(steering_storage->cruise_control_enabled);
    return STATUS_CODE_INVALID_ARGS;
  }

  ButtonState up = steering_storage->button_manager->buttons[STEERING_BUTTON_CRUISE_CONTROL_UP].state;
  ButtonState down = steering_storage->button_manager->buttons[STEERING_BUTTON_CRUISE_CONTROL_DOWN].state;

  /* Enable / Disable */

  if (!cruise_control_enabled_released && !up && !down) {
    cruise_control_enabled_released = true;
    return STATUS_CODE_OK;
  } else if (cruise_control_enabled_released && up && down) {
    cruise_control_enabled_released = false;

    if (drive_state_from_front != VEHICLE_DRIVE_STATE_DRIVE && drive_state_from_front != VEHICLE_DRIVE_STATE_CRUISE) {
#if (CC_DEBUG == 1)
      LOG_DEBUG("not in drive\r\n");
#endif
      return STATUS_CODE_INVALID_ARGS;
    }

    // Cruise control should start from our current speed
    int16_t current_speed_kmh_from_front_signed = (int16_t)get_motor_velocity_vehicle_velocity();

    if (current_speed_kmh_from_front_signed < 0) {
      return STATUS_CODE_INVALID_ARGS;
    }

    uint16_t current_speed_kmh_from_front = current_speed_kmh_from_front_signed;

    if (current_speed_kmh_from_front >= steering_storage->config->cruise_min_speed_kmh && current_speed_kmh_from_front <= steering_storage->config->cruise_max_speed_kmh) {
      steering_storage->cruise_control_target_speed_kmh = current_speed_kmh_from_front;
    } else {
#if (CC_DEBUG == 1)
      LOG_DEBUG("current speed is %u\r\n", current_speed_kmh_from_front_signed);
#endif
      return STATUS_CODE_INVALID_ARGS;
    }

    steering_storage->cruise_control_enabled = !steering_storage->cruise_control_enabled;

    // CAN TX
    set_steering_buttons_cruise_control(steering_storage->cruise_control_enabled);

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

  // CAN TX
  set_steering_target_velocity_cruise_control_target_velocity(steering_storage->cruise_control_target_speed_kmh);

  return STATUS_CODE_OK;
}
