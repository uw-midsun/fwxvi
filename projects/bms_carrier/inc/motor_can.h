#pragma once

/************************************************************************************************
 * @file   motor_can.h
 *
 * @brief  Source code for Motor controller CAN
 *
 * @date   2025-01-30
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "global_enums.h"

/* Intra-component Headers */

/**
 * @defgroup motor_controller
 * @brief    motor_controller Firmware
 * @{
 */

// SPI commands: Table 12-1
// requires mcp2515
// mcp2515 can be initialized before or after motor controller can

#define CURRENT_SCALE 100
#define VOLTAGE_SCALE 100
#define VELOCITY_SCALE 100
#define TEMP_SCALE 100

// also used as the current for regen braking, might need to be seperated
#define ACCERLATION_FORCE 1
#define CRUISE_THROTTLE_THRESHOLD 0
#define TORQUE_CONTROL_VEL 20000  // unobtainable rpm for current control

#define MATH_PI 3.14
#define WHEEL_DIAMETER 0.57147
#define VEL_TO_RPM_RATIO (60 / (2 * MATH_PI) * WHEEL_DIAMETER)  // TODO: set actual ratio, m/s to motor (rpm for m/s)
#define CONVERT_VELOCITY_TO_KPH 13234
// wheel diameter 557mm
// 1000 / (557 * pi) = 0.57147

#define DRIVER_CONTROL_BASE 0x500
#define MOTOR_CONTROLLER_BASE_L 0x400
#define MOTOR_CONTROLLER_BASE_R 0x80  // TODO: set to actual values

typedef enum MotorControllerMessageIds {
  IDENTIFICATION = 0x00,
  STATUS,
  BUS_MEASUREMENT,
  VEL_MEASUREMENT,
  PHASE_CURRENT,
  MOTOR_VOLTAGE,
  MOTOR_CURRENT,
  MOTOR_BACK_EMF,
  RAIL_15V,
  RAIL_3V_9V,
  RESERVED,
  HEAT_SINK_MOTOR_TEMP,
  DSP_BOARD_TEMP,
  RESERVED_2,
  ODOMETER_BUS_AMPHOUR,
  SLIP_SPEED = 0x17,
} MotorControllerMessageIds;

void init_motor_controller_can();
float prv_get_float(uint32_t u);
float clamp_float(float value);
void prv_update_target_current_velocity();

/** @} */
