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
 * @defgroup GlobalEnums
 * @brief    Global Enum definition for sharing between projects
 * @{
 */


// This file stores enums and defines which are exported between projects to allow both
// sides to use the same enums when sending and receiving CAN Messages over the
// primary network. These will typically take a value of either a STATE (value represents
// one of range of value) or a MASK (value represents a bit position in the field)
// To make things easier all enums in this file must follow a
// slightly modified naming convention.

/************************************************************************************************
 * Shared Global definitions
 ************************************************************************************************/

typedef enum {
  NEUTRAL,      /**< Neutral state */
  DRIVE,        /**< Drive state */
  REVERSE,      /**< Reverse state */

  /* Extra drive state types used only by motor */

  CRUISE,       /**< Cruise Control state */
  BRAKE,        /**< Brake state */
} DriveState;

/************************************************************************************************
 * BMS Global Definitions
 ************************************************************************************************/

/**
 * @brief   Battery relay states
 */
typedef enum {
  BMS_RELAY_STATE_CLOSE = 0,
  BMS_RELAY_STATE_FAULT,
  NUM_EE_RELAY_STATES,
} BmsRelayState;

/**
 * @brief   Battery fault definitions
 */
typedef enum {
  BMS_FAULT_OVERVOLTAGE,
  BMS_FAULT_UNBALANCE,
  BMS_FAULT_OVERTEMP_AMBIENT,
  BMS_FAULT_COMMS_LOSS_AFE,
  BMS_FAULT_COMMS_LOSS_CURR_SENSE,
  BMS_FAULT_OVERTEMP_CELL,
  BMS_FAULT_OVERCURRENT,
  BMS_FAULT_UNDERVOLTAGE,
  BMS_FAULT_KILLSWITCH,
  BMS_FAULT_RELAY_CLOSE_FAILED,
  BMS_FAULT_DISCONNECTED
} BmsFault;

#define BMS_FAULT_OVERVOLTAGE_MASK            (1U << BMS_FAULT_OVERVOLTAGE)
#define BMS_FAULT_UNBALANCE_MASK              (1U << BMS_FAULT_UNBALANCE)
#define BMS_FAULT_OVERTEMP_AMBIENT_MASK       (1U << BMS_FAULT_OVERTEMP_AMBIENT)
#define BMS_FAULT_COMMS_LOSS_AFE_MASK         (1U << BMS_FAULT_COMMS_LOSS_AFE)
#define BMS_FAULT_COMMS_LOSS_CURR_SENSE_MASK  (1U << BMS_FAULT_COMMS_LOSS_CURR_SENSE)
#define BMS_FAULT_OVERTEMP_CELL_MASK          (1U << BMS_FAULT_OVERTEMP_CELL)
#define BMS_FAULT_OVERCURRENT_MASK            (1U << BMS_FAULT_OVERCURRENT)
#define BMS_FAULT_UNDERVOLTAGE_MASK           (1U << BMS_FAULT_UNDERVOLTAGE)
#define BMS_FAULT_KILLSWITCH_MASK             (1U << BMS_FAULT_KILLSWITCH)
#define BMS_FAULT_RELAY_CLOSE_FAILED_MASK     (1U << BMS_FAULT_RELAY_CLOSE_FAILED)
#define BMS_FAULT_DISCONNECTED_MASK           (1U << BMS_FAULT_DISCONNECTED)

#define BMS_FAULT_HIGH_PRIORITY_MASK          (1U << 15U)
#define BMS_FAULT_LOW_PRIORITY_MASK           (1U << 14U)

/************************************************************************************************
 * Centre Console Global Definitions
 ************************************************************************************************/

// STEERING SIGNALS
// Signals for analog inputs received at the steering board
typedef enum {
  CC_STEERING_LIGHTS_OFF_STATE = 0,
  CC_STEERING_LIGHTS_LEFT_STATE,
  CC_STEERING_LIGHTS_RIGHT_STATE,
  NUM_EE_STEERING_LIGHTS,
} CCSteeringLights;

// Signals for digital inputs sent from the steering board
typedef enum {
  CC_STEERING_CRUISE_CONTROL_DECREASE_BIT = 0,  // Signal to decrease cruise control speed
  CC_STEERING_CRUISE_CONTROL_INCREASE_BIT,      // Signal to increase cruise control speed
  CC_STEERING_CRUISE_CONTROL_TOGGLE_BIT,        // Toggle cruise control on/off
} CCSteeringCruiseControl;

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

/************************************************************************************************
 * Power Distribution Global Definitions
 ************************************************************************************************/

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

/** @} */
