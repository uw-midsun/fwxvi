#pragma once

/************************************************************************************************
 * @file    power_ouputs.h
 *
 * @brief   Power Ouputs
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
 * @defgroup Front_Controller
 * @brief    Front Controller Board Firmware
 * @{
 */

/**
 * @brief   Power outputs
 * @details The order of this enum matters. It matches the schematic mux indices
 */
typedef enum {
  HORN = 0,
  SPARE_1,
  BRAKE_LIGHT,
  BPS_LIGHT,
  TELEM,
  REV_CAM,
  DRIVER_FAN,
  RIGHT_SIG,
  STEERING,
  LEFT_SIG,
  NUM_OUTPUTS,
} OutputId;

/**
 * @brief   Power output groups
 */
typedef enum {
  OUTPUT_GROUP_ALL = 0,
  IDLE_GROUP,
  LEFT_LIGHTS_GROUP,
  RIGHT_LIGHTS_GROUP,
  HAZARD_LIGHTS_GROUP,
  BPS_LIGHTS_GROUP,
  BRAKE_LIGHTS_GROUP,
  HORN_GROUP,
  NUM_OUTPUT_GROUPS,
} OutputGroup;

typedef struct {
  uint8_t num_outputs;           /**< How many outputs are in each group */
  OutputId outputs[NUM_OUTPUTS]; /**< Array for which outputs belong to each group */
} OutputGroupDef;

/** @} */
