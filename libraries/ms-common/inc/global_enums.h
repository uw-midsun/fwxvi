#pragma once

/************************************************************************************************
 * @file   global_enums.h
 *
 * @brief  Header file for global enums which are exported between projects with CAN
 *
 * @date   2025-01-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup bms_carrier
 * @brief    bms_carrier Firmware
 * @{
 */


// This file stores enums and defines which are exported between projects to allow both
// sides to use the same enums when sending and receiving CAN Messages over the
// primary network. These will typically take a value of either a STATE (value represents
// one of range of value) or a MASK (value represents a bit position in the field)
// To make things easier all enums in this file must follow a
// slightly modified naming convention.
//
// Example:
// typedef enum {
//   EE_<MY_CAN_MESSAGE_NAME>_<FIELD_NAME>_<VALUE> = 0,
//   // ...
//   NUM_EE_<MY_CAN_MESSAGE_NAME>_<FIELD_NAME>_<PLURAL>,
// } <MyCanMessageName><FieldName>

// Common Defines
typedef enum {
  EE_RELAY_STATE_CLOSE = 0,
  EE_RELAY_STATE_FAULT,
  NUM_EE_RELAY_STATES,
} EERelayState;

// STEERING SIGNALS
// Signals for analog inputs received at the steering board
typedef enum SteeringInfoAnalog {
  EE_STEERING_LIGHTS_OFF_STATE = 0,  // Turn lights off
  EE_STEERING_LIGHTS_LEFT_STATE,     // Left Turn Signal
  EE_STEERING_LIGHTS_RIGHT_STATE,    // Right Turn Signal
  NUM_EE_STEERING_LIGHTS,
} SteeringInfoAnalog;

// Signals for digital inputs sent from the steering board
typedef enum SteeringInfoDigitalMask {
  EE_STEERING_CC_DECREASE_BIT = 0,  // Signal to decrease cruise control speed
  EE_STEERING_CC_INCREASE_BIT,      // Signal to increase cruise control speed
  EE_STEERING_CC_TOGGLE_BIT,        // Toggle cruise control on/off
} SteeringInfoDigitalMask;

#define EE_STEERING_CC_INCREASE_MASK (1 << EE_STEERING_CC_INCREASE_BIT)
#define EE_STEERING_CC_DECREASE_MASK (1 << EE_STEERING_CC_DECREASE_BIT)
#define EE_STEERING_CC_TOGGLE_MASK (1 << EE_STEERING_CC_TOGGLE_BIT)

// CENTRE CONSOLE SIGNALS
typedef enum {
  EE_DRIVE_OUTPUT_CC_OFF_STATE = 0,  // Cruise control enabled,
  EE_DRIVE_OUTPUT_CC_ON_STATE,       // Cruise control disabled
  NUM_EE_DRIVE_OUTPUT_CC_STATES,
} CentreConsoleCCState;

typedef enum {
  EE_DRIVE_OUTPUT_REGEN_OFF_STATE = 0,  // Regen Braking enabled,
  EE_DRIVE_OUTPUT_REGEN_ON_STATE,       // Regen Braking disabled
  NUM_EE_DRIVE_OUTPUT_REGEN_STATES,
} CentreConsoleRegenState;

typedef enum {
  EE_DRIVE_OUTPUT_NEUTRAL_STATE = 0,
  EE_DRIVE_OUTPUT_DRIVE_STATE,
  EE_DRIVE_OUTPUT_REVERSE_STATE,
  NUM_EE_DRIVE_OUTPUT_STATES,
} CentreConsoleDriveState;

typedef enum {
  EE_DRIVE_FSM_BEGIN_PRECHARGE_OFF = 0,  // Begin precharge signal off
  EE_DRIVE_FSM_BEGIN_PRECHARGE_ON,       // Begin precharge signal on
  NUM_DRIVE_FSM_BEGIN_PRECHARGE_STATES,
} CentreConsoleBeginPrechargeState;

// POWER DISTRIBUTION (TODO)
typedef enum {
  EE_POWER_OFF_STATE = 0,
  EE_POWER_PRECHARGE_STATE,
  EE_POWER_DRIVE_STATE,
  EE_POWER_FAULT_STATE
} PowerDistributionPowerState;

typedef enum {
  EE_PWR_SEL_STATUS_PWR_SUPPLY_MASK = 0,  // Mask for bit indicating power supply status
  EE_PWR_SEL_STATUS_DCDC_MASK,            // Mask for bit indicating dcdc status
  EE_PWR_SEL_STATUS_AUX_MASK,             // Mask for bit indicating aux status
  NUM_EE_PWR_SEL_STATUS_MASKS,
} PowerSelectStatusMasks;

typedef enum {
  EE_PWR_SEL_FAULT_PWR_SUPPLY_OVERCURRENT_BIT = 0,
  EE_PWR_SEL_FAULT_PWR_SUPPY_OVERVOLTAGE_BIT,
  EE_PWR_SEL_FAULT_DCDC_OVERTEMP_BIT,
  EE_PWR_SEL_FAULT_DCDC_OVERCURRENT_BIT,
  EE_PWR_SEL_FAULT_DCDC_OVERVOLTAGE_BIT,
  EE_PWR_SEL_FAULT_AUX_OVERTEMP_BIT,
  EE_PWR_SEL_FAULT_AUX_OVERCURRENT_BIT,
  EE_PWR_SEL_FAULT_AUX_OVERVOLTAGE_BIT,
} PowerSelectFaultMasks;

typedef enum {
  EE_PD_STATUS_FAULT_BITSET_AUX_FAULT_BIT = 0,
  EE_PD_STATUS_FAULT_BITSET_DCDC_FAULT_BIT,
  NUM_EE_PD_STATUS_FAULT_BITSET_BITS,
} PdStatusFaultBitsetMasks;

#define EE_PWR_SEL_FAULT_PWR_SUPPLY_OVERCURRENT_MASK \
  (1 << EE_PWR_SEL_FAULT_PWR_SUPPLY_OVERCURRENT_BIT)

#define EE_PWR_SEL_FAULT_PWR_SUPPY_OVERVOLTAGE_MASK \
  (1 << EE_PWR_SEL_FAULT_PWR_SUPPY_OVERVOLTAGE_BIT)

#define EE_PWR_SEL_FAULT_DCDC_OVERTEMP_MASK (1 << EE_PWR_SEL_FAULT_DCDC_OVERTEMP_BIT)

#define EE_PWR_SEL_FAULT_DCDC_OVERCURRENT_MASK (1 << EE_PWR_SEL_FAULT_DCDC_OVERCURRENT_BIT)

#define EE_PWR_SEL_FAULT_DCDC_OVERVOLTAGE_MASK (1 << EE_PWR_SEL_FAULT_DCDC_OVERVOLTAGE_MASK)

#define EE_PWR_SEL_FAULT_AUX_OVERTEMP_MASK (1 << EE_PWR_SEL_FAULT_AUX_OVERTEMP_BIT)

#define EE_PWR_SEL_FAULT_AUX_OVERCURRENT_MASK (1 << EE_PWR_SEL_FAULT_AUX_OVERCURRENT_BIT)

#define EE_PWR_SEL_FAULT_AUX_OVERVOLTAGE_MASK (1 << EE_PWR_SEL_FAULT_AUX_OVERVOLTAGE_BIT)

#define EE_PD_STATUS_FAULT_BITSET_AUX_FAULT_MASK (1 << EE_PD_STATUS_FAULT_BITSET_AUX_FAULT_BIT)

#define EE_PD_STATUS_FAULT_BITSET_DCDC_FAULT_MASK (1 << EE_PD_STATUS_FAULT_BITSET_DCDC_FAULT_BIT)

// BMS SIGNALS
typedef enum EEBatteryHeartbeatFaultSource {
  EE_BPS_FAULT_SOURCE_KILLSWITCH = 0,
  EE_BPS_FAULT_SOURCE_AFE_CELL,
  EE_BPS_FAULT_SOURCE_AFE_TEMP,
  EE_BPS_FAULT_SOURCE_AFE_FSM,
  EE_BPS_FAULT_SOURCE_RELAY,
  EE_BPS_FAULT_SOURCE_CURRENT_SENSE,
  EE_BPS_FAULT_SOURCE_ACK_TIMEOUT,
  NUM_EE_BPS_FAULT_SOURCES,
} EEBatteryHeartbeatFaultSource;

// Battery heartbeat bitset representing fault reason
typedef uint8_t EEBatteryHeartbeatState;
#define EE_BPS_STATE_OK 0x0
#define EE_BPS_STATE_FAULT_KILLSWITCH (1 << EE_BPS_FAULT_SOURCE_KILLSWITCH)
#define EE_BPS_STATE_FAULT_AFE_CELL (1 << EE_BPS_FAULT_SOURCE_AFE_CELL)
#define EE_BPS_STATE_FAULT_AFE_TEMP (1 << EE_BPS_FAULT_SOURCE_AFE_TEMP)
#define EE_BPS_STATE_FAULT_AFE_FSM (1 << EE_BPS_FAULT_SOURCE_AFE_FSM)
#define EE_BPS_STATE_FAULT_RELAY (1 << EE_BPS_FAULT_SOURCE_RELAY)
#define EE_BPS_STATE_FAULT_CURRENT_SENSE (1 << EE_BPS_FAULT_SOURCE_CURRENT_SENSE)
#define EE_BPS_STATE_FAULT_ACK_TIMEOUT (1 << EE_BPS_FAULT_SOURCE_ACK_TIMEOUT)

/** @} */
