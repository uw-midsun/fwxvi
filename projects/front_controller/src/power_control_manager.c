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

// Mapping table to connect output enum ID to its hardware pin
static const GpioAddress output_pins[NUM_OUTPUTS] {
  [HEADLIGHT] = {.port = GPIO_PORT_A, .pin = 1},
  [LEFT_SIG] = { .port = GPIO_PORT_A, .pin = 8 },
  [RIGHT_SIG] = { .port = GPIO_PORT_A, .pin = 9 },
  [BRAKE_LIGHT] = { .port = GPIO_PORT_A, .pin = 10 },
  [BPS_LIGHT] = { .port = GPIO_PORT_A, .pin = 11 },
  [DRIVER_FAN] = { .port = GPIO_PORT_A, .pin = 12 },
  [REV_CAM] = { .port = GPIO_PORT_A, .pin = 15 },
  [TELEM] = { .port = GPIO_PORT_B, .pin = 2 },
  [STEERING] = { .port = GPIO_PORT_B, .pin = 3 },
  [HORN] = { .port = GPIO_PORT_B, .pin = 4 },
  [SPARE_1] = { .port = GPIO_PORT_B, .pin = 10 },
  [SPARE_2] = { .port = GPIO_PORT_B, .pin = 11 },
}

// Initializing all GPIO pins with correct address and low state
void power_control_manager_init() {
  for (uint8_t i = 0; i < NUM_OUTPUTS; i++) {
    gpio_init_pin(&output_pins[i], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  }
}

// Output Group Definitions
static const OutputGroupDef output_group_all = {
  .num_outputs = 12,
  .outputs = { HEADLIGHT, LEFT_SIG, RIGHT_SIG, BRAKE_LIGHT, BPS_LIGHT, DRIVER_FAN, REV_CAM, TELEM, STEERING, HORN, SPARE_1, SPARE_2 },
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
  [IDLE_GROUP] = &output_group_idle_group,     [LEFT_GROUP] = &output_group_left_group, [RIGHT_GROUP] = &output_group_right_group,
  [HAZARD_GROUP] = &output_group_hazard_group, [BPS_GROUP] = &output_group_bps_group,   [BRAKE_GROUP] = &output_group_brake_group,
};

// fix this, not sure if it wil lwork if you want to enable or disable a certain output
void power_control_set_output_group(OutputGroup group, bool enable) {

  if (group >= NUM_OUTPUT_GROUPS) {
    return;
  }

  OutputGroupDef *grp = output_group_map[group];

  if (grp == NULL) {
    return;
  }

  for (uint8_t i = 0; i < grp->num_outputs; i++) {

    OutputId id = grp->outputs[i];          // enum ID

    if (enable) {
      gpio_set_state(&output_pins[id], GPIO_STATE_HIGH);    // look up the pin, then set it
    } else if (!enable) {
      gpio_set_state(&output_pins[id], GPIO_STATE_LOW);
    }
  }
}


