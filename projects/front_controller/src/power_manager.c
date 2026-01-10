/************************************************************************************************
 * @file    power_manager.c
 *
 * @brief   Power Manager
 *
 * @date    2025-07-27
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"

/* Intra-component Headers */
#include "front_controller_hw_defs.h"
#include "front_controller_setters.h"
#include "power_manager.h"
#include "power_outputs.h"

/** @brief  Derived from data sheet */
#define POWER_MANAGER_HI_SIDE_CURRENT_SENSE_RATIO 3000
#define POWER_MANAGER_HI_SIDE_RESISTOR_OHMS 1000

/** @brief  Derived from data sheet */
#define POWER_MANAGER_LO_SIDE_CURRENT_SENSE_RATIO 1000
#define POWER_MANAGER_LO_SIDE_RESISTOR_OHMS 10000

static PowerManagerStorage s_power_manager_storage = { 0U };

static FrontControllerStorage *front_controller_storage = NULL;

static GpioAddress MUX_SEL_0 = FRONT_CONTROLLER_MUX_SEL_0;
static GpioAddress MUX_SEL_1 = FRONT_CONTROLLER_MUX_SEL_1;
static GpioAddress MUX_SEL_2 = FRONT_CONTROLLER_MUX_SEL_2;
static GpioAddress MUX_SEL_3 = FRONT_CONTROLLER_MUX_SEL_3;
static GpioAddress MUX_OUT = FRONT_CONTROLLER_MUX_OUTPUT;

/************************************************************************************************
 * Mapping table to connect output enum ID to its hardware pin
 ************************************************************************************************/

const GpioAddress output_pins[NUM_OUTPUTS] = {
  [LEFT_SIG] = FRONT_CONTROLLER_LEFT_SIG_LS_ENABLE,   [RIGHT_SIG] = FRONT_CONTROLLER_RIGHT_SIG_LS_ENABLE,   [BRAKE_LIGHT] = FRONT_CONTROLLER_BRAKE_LIGHT_LS_ENABLE,
  [BPS_LIGHT] = FRONT_CONTROLLER_BPS_LIGHT_LS_ENABLE, [DRIVER_FAN] = FRONT_CONTROLLER_DRIVER_FAN_LS_ENABLE, [REV_CAM] = FRONT_CONTROLLER_REV_CAM_LS_ENABLE,
  [TELEM] = FRONT_CONTROLLER_TELEM_LS_ENABLE,         [STEERING] = FRONT_CONTROLLER_STEERING_LS_ENABLE,     [HORN] = FRONT_CONTROLLER_HORN_LS_ENABLE,
  [SPARE_1] = FRONT_CONTROLLER_SPARE_1_LS_ENABLE
};

/************************************************************************************************
 * Output group definitions
 ************************************************************************************************/

static OutputGroupDef output_group_all = {
  .num_outputs = NUM_OUTPUTS,
  .outputs = { LEFT_SIG, RIGHT_SIG, BRAKE_LIGHT, BPS_LIGHT, DRIVER_FAN, REV_CAM, TELEM, STEERING, HORN, SPARE_1 },
};

static OutputGroupDef output_group_active_group = {
  .num_outputs = 3,
  .outputs = { REV_CAM, TELEM, STEERING },
};

static OutputGroupDef output_group_left_lights_group = {
  .num_outputs = 1,
  .outputs = { LEFT_SIG },
};

static OutputGroupDef output_group_right_lights_group = {
  .num_outputs = 1,
  .outputs = { RIGHT_SIG },
};

static OutputGroupDef output_group_hazard_lights_group = {
  .num_outputs = 2,
  .outputs = { LEFT_SIG, RIGHT_SIG },
};

static OutputGroupDef output_group_bps_lights_group = {
  .num_outputs = 1,
  .outputs = { BPS_LIGHT },
};

static OutputGroupDef output_group_brake_lights_group = {
  .num_outputs = 1,
  .outputs = { BRAKE_LIGHT },
};

static OutputGroupDef output_group_horn_group = {
  .num_outputs = 1,
  .outputs = { HORN },
};

OutputGroupDef *output_group_map[NUM_OUTPUT_GROUPS] = {
  [OUTPUT_GROUP_ALL] = &output_group_all,
  [OUTPUT_GROUP_ACTIVE] = &output_group_active_group,
  [OUTPUT_GROUP_LEFT_LIGHTS] = &output_group_left_lights_group,
  [OUTPUT_GROUP_RIGHT_LIGHTS] = &output_group_right_lights_group,
  [OUTPUT_GROUP_HAZARD_LIGHTS] = &output_group_hazard_lights_group,
  [OUTPUT_GROUP_BPS_LIGHTS] = &output_group_bps_lights_group,
  [OUTPUT_GROUP_BRAKE_LIGHTS] = &output_group_brake_lights_group,
  [OUTPUT_GROUP_HORN] = &output_group_horn_group,
};

/************************************************************************************************
 * Private functions
 ************************************************************************************************/
static uint16_t power_sense_lo_current_calc(uint16_t sampled_voltage) {
  uint16_t result = (uint16_t)(((float)sampled_voltage / (float)POWER_MANAGER_LO_SIDE_RESISTOR_OHMS) * (float)POWER_MANAGER_LO_SIDE_CURRENT_SENSE_RATIO);
  return result;
}

static uint16_t power_sense_hi_current_calc(uint16_t sampled_voltage) {
  uint16_t result = (uint16_t)(((float)sampled_voltage / (float)POWER_MANAGER_HI_SIDE_RESISTOR_OHMS) * (float)POWER_MANAGER_HI_SIDE_CURRENT_SENSE_RATIO);
  return result;
}

static void power_manager_set_telemetry() {
  set_fc_power_group_A_rev_cam_current(s_power_manager_storage.current_readings[REV_CAM]);
  set_fc_power_group_A_telem_current(s_power_manager_storage.current_readings[TELEM]);
  set_fc_power_group_A_steering_current(s_power_manager_storage.current_readings[STEERING]);
  set_fc_power_group_A_driver_fan_current(s_power_manager_storage.current_readings[DRIVER_FAN]);

  set_fc_power_group_B_horn_current(s_power_manager_storage.current_readings[HORN]);
  set_fc_power_group_B_spare_current(s_power_manager_storage.current_readings[SPARE_1]);

  set_fc_power_lights_group_brake_light_sig_current(s_power_manager_storage.current_readings[BRAKE_LIGHT]);
  set_fc_power_lights_group_bps_light_sig_current(s_power_manager_storage.current_readings[BPS_LIGHT]);
  set_fc_power_lights_group_right_sig_current(s_power_manager_storage.current_readings[RIGHT_SIG]);
  set_fc_power_lights_group_left_sig_current(s_power_manager_storage.current_readings[LEFT_SIG]);
}

/************************************************************************************************
 * Public functions
 ************************************************************************************************/

StatusCode power_manager_init(FrontControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  front_controller_storage = storage;
  front_controller_storage->power_manager_storage = &s_power_manager_storage;

  for (uint8_t i = 0; i < NUM_OUTPUTS; i++) {
    gpio_init_pin(&output_pins[i], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  }

  gpio_init_pin(&MUX_SEL_0, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&MUX_SEL_1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&MUX_SEL_2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&MUX_SEL_3, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  /* Initialize mux out as ADC pin */
  gpio_init_pin(&MUX_OUT, GPIO_ANALOG, GPIO_STATE_LOW);
  adc_add_channel(&MUX_OUT);
  adc_init();

  return STATUS_CODE_OK;
}

StatusCode power_manager_run_current_sense(OutputGroup group) {
  if (front_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (group > NUM_OUTPUT_GROUPS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  OutputGroupDef *mapped_group = output_group_map[group];

  for (uint16_t i = 0; i < mapped_group->num_outputs; i++) {
    /* Set mux select pins using the bits of i */
    gpio_set_state(&MUX_SEL_0, (i & 0x1U) ? GPIO_STATE_HIGH : GPIO_STATE_LOW);
    gpio_set_state(&MUX_SEL_1, (i & 0x2U) ? GPIO_STATE_HIGH : GPIO_STATE_LOW);
    gpio_set_state(&MUX_SEL_2, (i & 0x4U) ? GPIO_STATE_HIGH : GPIO_STATE_LOW);
    gpio_set_state(&MUX_SEL_3, (i & 0x8U) ? GPIO_STATE_HIGH : GPIO_STATE_LOW);

    adc_run();

    uint16_t sampled_voltage;
    adc_read_converted(&MUX_OUT, &sampled_voltage);

    if (i == HORN) {
      front_controller_storage->power_manager_storage->current_readings[i] = power_sense_hi_current_calc(sampled_voltage);
    } else {
      front_controller_storage->power_manager_storage->current_readings[i] = power_sense_lo_current_calc(sampled_voltage);
    }

    LOG_DEBUG("GROUP %d | ADC %d | CURRENT %d\r\n", i, sampled_voltage, s_power_manager_storage.current_readings[i]);

    delay_ms(10);
  }

  power_manager_set_telemetry();

  return STATUS_CODE_OK;
}

StatusCode power_manager_set_output_group(OutputGroup group, bool enable) {
  if (front_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (group > NUM_OUTPUT_GROUPS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  OutputGroupDef *mapped_group = output_group_map[group];

  for (uint8_t i = 0U; i < mapped_group->num_outputs; i++) {
    OutputId output_id = mapped_group->outputs[i];

    if (enable) {
      gpio_set_state(&output_pins[output_id], GPIO_STATE_HIGH);
    } else if (!enable) {
      gpio_set_state(&output_pins[output_id], GPIO_STATE_LOW);
    }
  }

  return STATUS_CODE_OK;
}

StatusCode power_manager_toggle_output_group(OutputGroup group) {
  if (front_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (group > NUM_OUTPUT_GROUPS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  OutputGroupDef *mapped_group = output_group_map[group];

  if (mapped_group == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  for (uint8_t i = 0U; i < mapped_group->num_outputs; i++) {
    OutputId output_id = mapped_group->outputs[i];
    gpio_toggle_state(&output_pins[output_id]);
  }

  return STATUS_CODE_OK;
}
