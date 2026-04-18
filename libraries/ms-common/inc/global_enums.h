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
  VEHICLE_DRIVE_STATE_INVALID = 0, /**< Wrong State */
  VEHICLE_DRIVE_STATE_NEUTRAL = 1, /**< Car Not Actively Moving in Neutral */
  VEHICLE_DRIVE_STATE_DRIVE = 2,   /**< Car Drive Forward */
  VEHICLE_DRIVE_STATE_REVERSE = 3, /**< Car Drive Reverse*/
  VEHICLE_DRIVE_STATE_CRUISE = 4,  /**< Car Cruise Constant Velocity */
  VEHICLE_DRIVE_STATE_BRAKE = 5,   /**< Car Brake */
  VEHICLE_DRIVE_STATE_REGEN = 6,   /**< Car Brake With Regen */
  VEHICLE_DRIVE_STATE_FAULT = 7,   /**< BPS fault */
  VEHICLE_DRIVE_NUM_STATES,
} VehicleDriveState;

/**
 * @brief LUT + Macro to help print vehicle drive states
 */
static const char *vehicle_drive_state_strings[VEHICLE_DRIVE_NUM_STATES] = {
  [VEHICLE_DRIVE_STATE_INVALID] = "INVALID", [VEHICLE_DRIVE_STATE_NEUTRAL] = "NEUTRAL", [VEHICLE_DRIVE_STATE_DRIVE] = "DRIVE", [VEHICLE_DRIVE_STATE_REVERSE] = "REVERSE",
  [VEHICLE_DRIVE_STATE_CRUISE] = "CRUISE",   [VEHICLE_DRIVE_STATE_BRAKE] = "BRAKE",     [VEHICLE_DRIVE_STATE_REGEN] = "REGEN",
};

#define VEHICLE_DRIVE_STATE_TO_STR(s) (((s) < VEHICLE_DRIVE_NUM_STATES) ? vehicle_drive_state_strings[(s)] : "UNKNOWN")

/**
 * @brief   Vehicle power states
 */
typedef enum {
  VEHICLE_POWER_STATE_IDLE,   /**< Ready but not driving or charging, cars in neutral */
  VEHICLE_POWER_STATE_DRIVE,  /**< Driving: motor relays closed, LV enabled */
  VEHICLE_POWER_STATE_CHARGE, /**< Charging: charger detected, motor relay may be required */
  VEHICLE_POWER_STATE_FAULT   /**< Faulted: relays open, latched until reset */
} VehiclePowerStates;

/**
 * @brief   Signal and BPS light blink period in milliseconds
 */
#define GLOBAL_SIGNAL_LIGHTS_BLINK_PERIOD_MS 400U
#define GLOBAL_BPS_LIGHTS_BLINK_PERIOD_MS 200U

/**
 * @brief   WS22 motor limit and error flags
 */

typedef enum {
  ERROR_FLAG_HARDWARE_OVERCURRENT = 0,
  ERROR_FLAG_SOFTWARE_OVERCURRENT = 1,
  ERROR_FLAG_DC_BUS_OV = 2,
  ERROR_FLAG_BAD_HALL_SEQUENCE = 3,
  ERROR_FLAG_WATCHDOG_CAUSED_RESET = 4,
  ERROR_FLAG_CFG_READ_ERROR = 6,
  ERROR_FLAG_UVLO = 6,
  ERROR_FLAG_DESATURATION_FAULT = 7,
  ERROR_FLAG_MOTOR_OVER_SPEED = 8,
  LIMIT_FLAG_OUTPUT_VOLTAGE_PWM = 9,
  LIMIT_FLAG_MOTOR_CURRENT = 10,
  LIMIT_FLAG_VELOCITY = 11,
  LIMIT_FLAG_BUS_CURRENT = 12,
  LIMIT_FLAG_BUS_VOLTAGE_UPPER = 13,
  LIMIT_FLAG_BUS_VOLTAGE_LOWER = 14,
  LIMIT_FLAG_TEMPERATURE = 15,
  NUM_WS22_MOTOR_FLAGS,
} Ws22MotorFlags;

/** @brief  Hardware overcurrent error flag mask */
#define ERROR_FLAG_HARDWARE_OVERCURRENT_MASK (1U << ERROR_FLAG_HARDWARE_OVERCURRENT)
/** @brief  Software overcurrent error flag mask */
#define ERROR_FLAG_SOFTWARE_OVERCURRENT_MASK (1U << ERROR_FLAG_SOFTWARE_OVERCURRENT)
/** @brief  DC bus overvoltage error flag mask */
#define ERROR_FLAG_DC_BUS_OV_MASK (1U << ERROR_FLAG_DC_BUS_OV)
/** @brief  Bad hall sequence error flag mask */
#define ERROR_FLAG_BAD_HALL_SEQUENCE_MASK (1U << ERROR_FLAG_BAD_HALL_SEQUENCE)
/** @brief  Watchdog caused reset error flag mask */
#define ERROR_FLAG_WATCHDOG_CAUSED_RESET_MASK (1U << ERROR_FLAG_WATCHDOG_CAUSED_RESET)
/** @brief  CFG read error flag mask */
#define ERROR_FLAG_CFG_READ_ERROR_MASK (1U << ERROR_FLAG_CFG_READ_ERROR)
/** @brief  UVLO error flag mask */
#define ERROR_FLAG_UVLO_MASK (1U << ERROR_FLAG_UVLO)
/** @brief  Desaturation fault error flag mask */
#define ERROR_FLAG_DESATURATION_FAULT_MASK (1U << ERROR_FLAG_DESATURATION_FAULT)
/** @brief  Motor overspeed error flag mask */
#define ERROR_FLAG_MOTOR_OVER_SPEED_MASK (1U << ERROR_FLAG_MOTOR_OVER_SPEED)
/** @brief  Output voltage PWM limit flag mask */
#define LIMIT_FLAG_OUTPUT_VOLTAGE_PWM_MASK (1U << LIMIT_FLAG_OUTPUT_VOLTAGE_PWM)
/** @brief  Motor current limit flag mask */
#define LIMIT_FLAG_MOTOR_CURRENT_MASK (1U << LIMIT_FLAG_MOTOR_CURRENT)
/** @brief  Velocity limit flag mask */
#define LIMIT_FLAG_VELOCITY_MASK (1U << LIMIT_FLAG_VELOCITY)
/** @brief  Bus current limit flag mask */
#define LIMIT_FLAG_BUS_CURRENT_MASK (1U << LIMIT_FLAG_BUS_CURRENT)
/** @brief  Bus voltage upper limit flag mask */
#define LIMIT_FLAG_BUS_VOLTAGE_UPPER_MASK (1U << LIMIT_FLAG_BUS_VOLTAGE_UPPER)
/** @brief  Bus voltage lower limit flag mask */
#define LIMIT_FLAG_BUS_VOLTAGE_LOWER_MASK (1U << LIMIT_FLAG_BUS_VOLTAGE_LOWER)
/** @brief  Temperature limit flag mask */
#define LIMIT_FLAG_TEMPERATURE_MASK (1U << LIMIT_FLAG_TEMPERATURE)

/** @brief  IDs of WS22 CAN messages to exclude from telemetry datagram forwarding */
#define IS_EXCLUDED_WS22_CAN_ID(x) ((((x) >= 128U) && ((x) <= 142U)) || ((x) == 151U) || ((x) == 1281U))


/************************************************************************************************
 * Front Controller Global Definitions
 ************************************************************************************************/

/**
 * @brief Stores data that defines when the pedal is full pressed or unpressed
 * @details The lower_value is the value at which the pedal is considered fully unpressed whereas the upper_value is the value at which the pedal is fully pressed
 */
typedef struct PedalCalibrationData {
  uint16_t lower_value; /**< ADC reading when the pedal is considered fully released */
  uint16_t upper_value; /**< ADC reading when the pedal is considered fully pressed */
} PedalCalibrationData;

/**
 * @brief  Pedal persist struct
 */
typedef struct __attribute__((aligned(4))) PedalPersistData {
  PedalCalibrationData accel_pedal_data_raw;
  PedalCalibrationData accel_pedal_data_amplified;
  PedalCalibrationData brake_pedal_data;
} PedalPersistData;

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
  BPS_FAULT_DISCONNECTED,          /**< BPS Disconnected fault */
  NUM_BPS_FAULTS
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
  STEERING_LIGHTS_OFF_STATE = 0,    /**< Steering lights off */
  STEERING_LIGHTS_LEFT_STATE = 1,   /**< Steering light left turn */
  STEERING_LIGHTS_RIGHT_STATE = 2,  /**< Steering light right turn */
  STEERING_LIGHTS_HAZARD_STATE = 3, /**< Steering light hazard state */
  STEERING_LIGHTS_NUM_STATES,       /**< Number of steering light states */
} SteeringLightState;

/**
 * @brief LUT + Macro to help print steering light states
 */
static const char *steering_light_state_strings[STEERING_LIGHTS_NUM_STATES] = { "OFF", "LEFT", "RIGHT", "HAZARD" };

#define STEERING_LIGHT_STATE_TO_STR(s) (((s) < STEERING_LIGHTS_NUM_STATES) ? steering_light_state_strings[(s)] : "UNKNOWN")

typedef enum {
  BPS_LIGHT_ON_STATE,   /**< BPS light on state */
  BPS_LIGHT_OFF_STATE,  /**< BPS light off state */
  NUM_BPS_LIGHT_STATES, /**< Number of BPS light states */
} BpsLightState;

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
