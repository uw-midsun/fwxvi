/************************************************************************************************
 * @file    power_control_manager.c
 *
 * @brief   Power Control Manager
 *
 * @date    2025-07-27
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
/* Inter-component Headers */
#include "gpio.h"

/* Intra-component Headers */
#include "power_control_manager.h"


// Defines to map GPIO address
static const GpioAddress HEADLIGHT = { .port = GPIO_PORT_A, .pin = 1 };
static const GpioAddress LEFT_SIG =  { .port = GPIO_PORT_A, .pin = 8 };
static const GpioAddress RIGHT_SIG =  { .port = GPIO_PORT_A, .pin = 9 };
static const GpioAddress BRAKE_LIGHT =  { .port = GPIO_PORT_A, .pin =10 };
static const GpioAddress BPS_LIGHT = { .port = GPIO_PORT_A, .pin = 11 };
static const GpioAddress DRIVER_FAN = { .port = GPIO_PORT_A, .pin = 12 };
static const GpioAddress REV_CAM = { .port = GPIO_PORT_A, .pin = 15 };
static const GpioAddress TELEM = { .port = GPIO_PORT_B, .pin = 2 };
static const GpioAddress STEERING = { .port = GPIO_PORT_B, .pin = 3};
static const GpioAddress HORN = { .port = GPIO_PORT_B, .pin = 4 };
static const GpioAddress SPARE_1 = { .port = GPIO_PORT_B, .pin = 10 };
static const GpioAddress SPARE_2 = { .port = GPIO_PORT_B, .pin = 11 };

// Initializing all GPIO pins with correct address and low state
void power_control_manager_init() {
  gpio_init_pin(&HEADLIGHT, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&LEFT_SIG, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&RIGHT_SIG, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&BRAKE_LIGHT, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&BPS_LIGHT, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&DRIVER_FAN, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&REV_CAM, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&TELEM, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&STEERING, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&HORN, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&SPARE_1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&SPARE_2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
}

// Output Group Definitions
// !!!!! ADDED OUTPUT GROUP ALL STRUCT
static const OutputGroupDef output_group_all = {
  .num_outputs = 12,
  .outputs = {HEADLIGHT, LEFT_SIG, RIGHT_SIG, BRAKE_LIGHT, BPS_LIGHT, DRIVER_FAN, REV_CAM, TELEM, STEERING, HORN, SPARE_1, SPARE_2},
};

static OutputGroupDef output_group_idle_group = {
  .num_outputs = 4,
  .outputs = { REV_CAM, TELEM, STEERING, HEADLIGHT },
};

static OutputGroupDef output_group_left_group = {
  .num_outputs = 1,
  .outputs = { LEFT_SIG },
};

static OutputGroupDef output_group_right_group = {
  .num_outputs = 1,
  .outputs = { RIGHT_SIG },
};

static OutputGroupDef output_group_hazard_group = {
  .num_outputs = 2,
  .outputs = { LEFT_SIG, RIGHT_SIG },
};

static OutputGroupDef output_group_bps_group = {
  .num_outputs = 1,
  .outputs = { BPS_LIGHT },
};

static OutputGroupDef output_group_brake_group = {
  .num_outputs = 1,
  .outputs = { BRAKE_LIGHT },
};

static OutputGroupDef output_group_horn_group = {
  .num_outputs = 1,
  .outputs = { HORN },
};

// Global array definition
const OutputGroupDef *output_group_map[NUM_OUTPUT_GROUPS] = {
  [OUTPUT_GROUP_ALL] = &output_group_all,  // special case WAS NULL BEFORE !!!!! 
  [IDLE_GROUP] = &output_group_idle_group,
  [LEFT_GROUP] = &output_group_left_group,
  [RIGHT_GROUP] = &output_group_right_group,
  [HAZARD_GROUP] = &output_group_hazard_group,
  [BPS_GROUP] = &output_group_bps_group,
  [BRAKE_GROUP] = &output_group_brake_group,
};

// Enable/disable all outputs at once or certain outputs 
void power_control_set_output_group(OutputGroup group, bool enable) {
  if (group >= NUM_OUTPUT_GROUPS) {
    return;
  }

  GpioState state;
  if (enable) {
    state = GPIO_STATE_HIGH;
  } else {
    state = GPIO_STATE_LOW;
  }

  // Special case to enable/disable all the outputs at once
  if (group == OUTPUT_GROUP_ALL) {
    gpio_set_state(&HEADLIGHT, state);
    gpio_set_state(&LEFT_SIG, state);
    gpio_set_state(&RIGHT_SIG, state);
    gpio_set_state(&BRAKE_LIGHT, state);
    gpio_set_state(&BPS_LIGHT, state);
    gpio_set_state(&DRIVER_FAN, state);
    gpio_set_state(&REV_CAM, state);
    gpio_set_state(&TELEM, state);
    gpio_set_state(&STEERING, state);
    gpio_set_state(&HORN, state);

  } else {
    // Get the specific group
    OutputGroupDef *grp = output_group_map[group];

    // Error handling
    if (grp == NULL) {
      return;
    }

    // Iterate through its set of outputs and enable/disable the output
    for (uint8_t i = 0; i < grp->num_outputs; i++) {
      GpioAddress output = grp->outputs[i];
      if (output == HEADLIGHT) {  // NEED TO CHANGE THIS, you can't compare struct to struct. aryan said could do output.port && output.pin == headlight.port && headlight.pin but not the best method. find another work around
        gpio_set_state(&HEADLIGHT, state);
      } else if (output == LEFT_SIG) {
        gpio_set_state(&LEFT_SIG, state);
      } else if (output == RIGHT_SIG) {
        gpio_set_state(&RIGHT_SIG, state);
      } else if (output == BRAKE_LIGHT) {
        gpio_set_state(&BRAKE_LIGHT, state);
      } else if (output == BPS_LIGHT) {
        gpio_set_state(&BPS_LIGHT, state);
      } else if (output == DRIVER_FAN) {
        gpio_set_state(&DRIVER_FAN, state);
      } else if (output == REV_CAM) {
        gpio_set_state(&REV_CAM, state);
      } else if (output == TELEM) {
        gpio_set_state(&REV_CAM, state);
      } else if (output == STEERING) {
        gpio_set_state(&STEERING, state);
      } else if (output == HORN) {
        gpio_set_state(&HORN, state);
      }
    }
  }
}
