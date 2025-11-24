#pragma once

/************************************************************************************************
 * @file    power_control_ouputs.h
 *
 * @brief   Power Control Ouputs
 *
 * @date    2025-07-27
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "gpio.h"
#include "i2c.h"

/* Intra-component Headers */

/**
 * @defgroup power_control_ouputs
 * @brief    power_control_ouputs Firmware
 * @{
 */

// Enum for the output groups

typedef enum {
  LEFT_SIG = 0,
  RIGHT_SIG,
  BRAKE_LIGHT,
  BPS_LIGHT,
  DRIVER_FAN,
  REV_CAM,
  TELEM,
  STEERING,
  HORN,
  SPARE_1,
  NUM_OUTPUTS,
} OutputId;

typedef enum {
  OUTPUT_GROUP_ALL = 0,
  IDLE_GROUP,
  LEFT_GROUP,
  RIGHT_GROUP,
  HAZARD_GROUP,
  BPS_GROUP,
  BRAKE_GROUP,
  NUM_OUTPUT_GROUPS,
} OutputGroup;

typedef struct OutputGroupDef {
  uint8_t num_outputs;   // how many outputs are in each group
  OutputId outputs[12];  // array for which outputs belong to each group
} OutputGroupDef;

extern const GpioAddress output_pins[NUM_OUTPUTS];
extern const OutputGroupDef *output_group_map[NUM_OUTPUT_GROUPS];
/** @} */
