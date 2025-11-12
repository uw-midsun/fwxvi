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
#include "front_controller_hw_defs.h"
#include "power_control_manager.h"
#include "power_control_outputs.h"

// Mapping table to connect output enum ID to its hardware pin
const GpioAddress output_pins[NUM_OUTPUTS] = {
  [HEADLIGHT] = FRONT_CONTROLLER_HEADLIGHT_LS_ENABLE, [LEFT_SIG] = FRONT_CONTROLLER_LEFT_SIG_LS_ENABLE,
  [RIGHT_SIG] = FRONT_CONTROLLER_RIGHT_SIG_LS_ENABLE, [BRAKE_LIGHT] = FRONT_CONTROLLER_BRAKE_LIGHT_LS_ENABLE,
  [BPS_LIGHT] = FRONT_CONTROLLER_BPS_LIGHT_LS_ENABLE, [DRIVER_FAN] = FRONT_CONTROLLER_DRIVER_FAN_LS_ENABLE,
  [REV_CAM] = FRONT_CONTROLLER_REV_CAM_LS_ENABLE,     [TELEM] = FRONT_CONTROLLER_TELEM_LS_ENABLE,
  [STEERING] = FRONT_CONTROLLER_STEERING_LS_ENABLE,   [HORN] = FRONT_CONTROLLER_HORN_LS_ENABLE,
  [SPARE_1] = FRONT_CONTROLLER_SPARE_1_LS_ENABLE,     [SPARE_2] = FRONT_CONTROLLER_SPARE_2_LS_ENABLE,
};

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
  OutputGroupDef *grp = output_group_map[group];

  if (grp == NULL) {
    return;
  }

  for (uint8_t i = 0; i < grp->num_outputs; i++) {
    OutputId id = grp->outputs[i];  // enum ID

    if (enable) {
      gpio_set_state(&output_pins[id], GPIO_STATE_HIGH);  // look up the pin, then set it
    } else if (!enable) {
      gpio_set_state(&output_pins[id], GPIO_STATE_LOW);
    }
  }
}
