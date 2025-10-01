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

/************************************************************************************************
 * Shared Global definitions
 ************************************************************************************************/

/**
 * @brief   Vehicle drive states
 */
typedef enum {
  VEHICLE_NEUTRAL, /**< Neutral state */
  VEHICLE_DRIVE,   /**< Drive state */
  VEHICLE_REVERSE, /**< Reverse state */

  /* Extra drive state types used only by motor */

  VEHICLE_CRUISE, /**< Cruise Control state */
  VEHICLE_BRAKE,  /**< Brake state */
} VehicleDriveState;

/**
 * @brief   Vehicle power states
 */
typedef enum {
  VEHICLE_POWER_STATE_IDLE,   /**< Ready but not driving or charging, cars in neutral */
  VEHICLE_POWER_STATE_DRIVE,  /**< Driving: motor relays closed, LV enabled */
  VEHICLE_POWER_STATE_CHARGE, /**< Charging: charger detected, motor relay may be required */
  VEHICLE_POWER_STATE_FAULT   /**< Faulted: relays open, latched until reset */
} VehiclePowerStates;

/************************************************************************************************
 * Rear Controller Global Definitions
 ************************************************************************************************/

/**
 * @brief   Battery fault definitions
 */
typedef enum {
  BPS_FAULT_OVERVOLTAGE,           /**< BPS Overvoltage fault */
  BPS_FAULT_UNBALANCE,             /**< BPS Unbalanced fault */
  BPS_FAULT_OVERTEMP_AMBIENT,      /**< BPS Ambient temperature fault */
  BPS_FAULT_COMMS_LOSS_AFE,        /**< BPS Communication lost with AFEs fault */
  BPS_FAULT_COMMS_LOSS_CURR_SENSE, /**< BPS Communication lost with current sense fault */
  BPS_FAULT_OVERTEMP_CELL,         /**< BPS Cell temperature fault */
  BPS_FAULT_OVERCURRENT,           /**< BPS Overcurrent fault */
  BPS_FAULT_UNDERVOLTAGE,          /**< BPS Undervoltage fault */
  BPS_FAULT_KILLSWITCH,            /**< BPS Killswitch fault */
  BPS_FAULT_RELAY_CLOSE_FAILED,    /**< BPS Relay failed to close fault */
  BPS_FAULT_DISCONNECTED           /**< BPS Disconnected fault */
} BpsFault;

/** @brief  BPS Overvoltage fault mask */
#define BPS_FAULT_OVERVOLTAGE_MASK (1U << BPS_FAULT_OVERVOLTAGE)
/** @brief  BPS Unbalanced fault mask */
#define BPS_FAULT_UNBALANCE_MASK (1U << BPS_FAULT_UNBALANCE)
/** @brief  BPS Ambient temperature fault mask */
#define BPS_FAULT_OVERTEMP_AMBIENT_MASK (1U << BPS_FAULT_OVERTEMP_AMBIENT)
/** @brief  BPS Communication lost with AFEs fault mask */
#define BPS_FAULT_COMMS_LOSS_AFE_MASK (1U << BPS_FAULT_COMMS_LOSS_AFE)
/** @brief  BPS Communication lost with current sense fault mask */
#define BPS_FAULT_COMMS_LOSS_CURR_SENSE_MASK (1U << BPS_FAULT_COMMS_LOSS_CURR_SENSE)
/** @brief  BPS Cell temperature fault mask */
#define BPS_FAULT_OVERTEMP_CELL_MASK (1U << BPS_FAULT_OVERTEMP_CELL)
/** @brief  BPS Overcurrent fault mask */
#define BPS_FAULT_OVERCURRENT_MASK (1U << BPS_FAULT_OVERCURRENT)
/** @brief  BPS Undervoltage fault mask */
#define BPS_FAULT_UNDERVOLTAGE_MASK (1U << BPS_FAULT_UNDERVOLTAGE)
/** @brief  BPS Killswitch fault mask */
#define BPS_FAULT_KILLSWITCH_MASK (1U << BPS_FAULT_KILLSWITCH)
/** @brief  BPS Relay failed to close fault mask */
#define BPS_FAULT_RELAY_CLOSE_FAILED_MASK (1U << BPS_FAULT_RELAY_CLOSE_FAILED)
/** @brief  BPS Disconnected fault mask */
#define BPS_FAULT_DISCONNECTED_MASK (1U << BPS_FAULT_DISCONNECTED)

/** @brief  BPS High priority fault mask */
#define BPS_FAULT_HIGH_PRIORITY_MASK (1U << 15U)
/** @brief  BPS Low priority fault mask */
#define BPS_FAULT_LOW_PRIORITY_MASK (1U << 14U)

/************************************************************************************************
 * Steering Global Definitions
 ************************************************************************************************/

/**
 * @brief   Steering lights definitions
 */
typedef enum {
  STEERING_LIGHTS_OFF_STATE,   /**< Steering lights off turn */
  STEERING_LIGHTS_LEFT_STATE,  /**< Steering light left turn */
  STEERING_LIGHTS_RIGHT_STATE, /**< Steering light right turn */
  NUM_STEERING_LIGHTS,         /**< Number of steering light states */
} SteeringLightState;

/**
 * @brief   Steering cruise control definitions
 */
typedef enum {
  STEERING_CRUISE_CONTROL_DECREASE_BIT = 0, /**< Decrease cruise control speed */
  STEERING_CRUISE_CONTROL_INCREASE_BIT,     /**< Increase cruise control speed */
  STEERING_CRUISE_CONTROL_TOGGLE_BIT,       /**< Toggle cruise control */
} SteeringCruiseControl;

/** @brief  Increase cruise control speed mask */
#define STEERING_CC_INCREASE_MASK (1U << EE_STEERING_CC_INCREASE_BIT)
/** @brief  Decrease cruise control speed mask */
#define STEERING_CC_DECREASE_MASK (1U << EE_STEERING_CC_DECREASE_BIT)
/** @brief  Toggle cruise control mask */
#define STEERING_CC_TOGGLE_MASK (1U << EE_STEERING_CC_TOGGLE_BIT)

/************************************************************************************************
 * Power Distribution Global Definitions
 ************************************************************************************************/

/**
 * @brief   Power distribution power state definitions
 */
typedef enum {
  EE_POWER_OFF_STATE,       /**< Power distribution off state */
  EE_POWER_PRECHARGE_STATE, /**< Power distribution precharge state */
  EE_POWER_DRIVE_STATE,     /**< Power distribution drive state */
  EE_POWER_FAULT_STATE      /**< Power distribution fault state */
} PDPowerState;

/** @} */
