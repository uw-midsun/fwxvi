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
} DriveState;

/************************************************************************************************
 * BMS Global Definitions
 ************************************************************************************************/

/**
 * @brief   Battery relay states
 */
typedef enum {
  BMS_RELAY_STATE_CLOSE, /**< BMS Relays closed */
  BMS_RELAY_STATE_FAULT, /**< BMS Relays fault */
  NUM_BMS_RELAY_STATES,  /**< Number of BMS Relay states */
} BmsRelayState;

/**
 * @brief   Battery fault definitions
 */
typedef enum {
  BMS_FAULT_OVERVOLTAGE,           /**< BMS Overvoltage fault */
  BMS_FAULT_UNBALANCE,             /**< BMS Unbalanced fault */
  BMS_FAULT_OVERTEMP_AMBIENT,      /**< BMS Ambient temperature fault */
  BMS_FAULT_COMMS_LOSS_AFE,        /**< BMS Communication lost with AFEs fault */
  BMS_FAULT_COMMS_LOSS_CURR_SENSE, /**< BMS Communication lost with current sense fault */
  BMS_FAULT_OVERTEMP_CELL,         /**< BMS Cell temperature fault */
  BMS_FAULT_OVERCURRENT,           /**< BMS Overcurrent fault */
  BMS_FAULT_UNDERVOLTAGE,          /**< BMS Undervoltage fault */
  BMS_FAULT_KILLSWITCH,            /**< BMS Killswitch fault */
  BMS_FAULT_RELAY_CLOSE_FAILED,    /**< BMS Relay failed to close fault */
  BMS_FAULT_DISCONNECTED           /**< BMS Disconnected fault */
} BmsFault;

/** @brief  BMS Overvoltage fault mask */
#define BMS_FAULT_OVERVOLTAGE_MASK (1U << BMS_FAULT_OVERVOLTAGE)
/** @brief  BMS Unbalanced fault mask */
#define BMS_FAULT_UNBALANCE_MASK (1U << BMS_FAULT_UNBALANCE)
/** @brief  BMS Ambient temperature fault mask */
#define BMS_FAULT_OVERTEMP_AMBIENT_MASK (1U << BMS_FAULT_OVERTEMP_AMBIENT)
/** @brief  BMS Communication lost with AFEs fault mask */
#define BMS_FAULT_COMMS_LOSS_AFE_MASK (1U << BMS_FAULT_COMMS_LOSS_AFE)
/** @brief  BMS Communication lost with current sense fault mask */
#define BMS_FAULT_COMMS_LOSS_CURR_SENSE_MASK (1U << BMS_FAULT_COMMS_LOSS_CURR_SENSE)
/** @brief  BMS Cell temperature fault mask */
#define BMS_FAULT_OVERTEMP_CELL_MASK (1U << BMS_FAULT_OVERTEMP_CELL)
/** @brief  BMS Overcurrent fault mask */
#define BMS_FAULT_OVERCURRENT_MASK (1U << BMS_FAULT_OVERCURRENT)
/** @brief  BMS Undervoltage fault mask */
#define BMS_FAULT_UNDERVOLTAGE_MASK (1U << BMS_FAULT_UNDERVOLTAGE)
/** @brief  BMS Killswitch fault mask */
#define BMS_FAULT_KILLSWITCH_MASK (1U << BMS_FAULT_KILLSWITCH)
/** @brief  BMS Relay failed to close fault mask */
#define BMS_FAULT_RELAY_CLOSE_FAILED_MASK (1U << BMS_FAULT_RELAY_CLOSE_FAILED)
/** @brief  BMS Disconnected fault mask */
#define BMS_FAULT_DISCONNECTED_MASK (1U << BMS_FAULT_DISCONNECTED)

/** @brief  BMS High priority fault mask */
#define BMS_FAULT_HIGH_PRIORITY_MASK (1U << 15U)
/** @brief  BMS Low priority fault mask */
#define BMS_FAULT_LOW_PRIORITY_MASK (1U << 14U)

/************************************************************************************************
 * Centre Console Global Definitions
 ************************************************************************************************/

/**
 * @brief   Steering lights definitions
 */
typedef enum {
  CC_STEERING_LIGHTS_OFF_STATE,   /**< Steering lights off turn */
  CC_STEERING_LIGHTS_LEFT_STATE,  /**< Steering light left turn */
  CC_STEERING_LIGHTS_RIGHT_STATE, /**< Steering light right turn */
  NUM_EE_STEERING_LIGHTS,         /**< Number of steering light states */
} CCSteeringLights;

/**
 * @brief   Steering cruise control definitions
 */
typedef enum {
  CC_STEERING_CRUISE_CONTROL_DECREASE_BIT = 0, /**< Decrease cruise control speed */
  CC_STEERING_CRUISE_CONTROL_INCREASE_BIT,     /**< Increase cruise control speed */
  CC_STEERING_CRUISE_CONTROL_TOGGLE_BIT,       /**< Toggle cruise control */
} CCSteeringCruiseControl;

/** @brief  Increase cruise control speed mask */
#define EE_STEERING_CC_INCREASE_MASK (1U << EE_STEERING_CC_INCREASE_BIT)
/** @brief  Decrease cruise control speed mask */
#define EE_STEERING_CC_DECREASE_MASK (1U << EE_STEERING_CC_DECREASE_BIT)
/** @brief  Toggle cruise control mask */
#define EE_STEERING_CC_TOGGLE_MASK (1U << EE_STEERING_CC_TOGGLE_BIT)

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
