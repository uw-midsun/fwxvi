#pragma once

/************************************************************************************************
 * @file   ws22_motor_can.h
 *
 * @brief  Header file for Wavesculptor 22 CAN interface
 *
 * @date   2025-06-29
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#include "can.h"
#include "status.h"

/* Intra-component Headers */
#include "front_controller.h"

/**
 * @defgroup Front_Controller
 * @brief    Front Controller Board Firmware
 * @{
 */

/* CAN Message Base Addresses */
#define WS22_CAN_BASE_CONTROL 0x500U /**< Base address for control messages */
#define WS22_CAN_BASE_STATUS 0x80U   /**< Base address for status messages */

/* CAN Message IDs */
#define WS22_CAN_ID_DRIVE_CMD (WS22_CAN_BASE_CONTROL + 0x01U)
#define WS22_CAN_ID_STATUS_INFO (WS22_CAN_BASE_STATUS + 0x01U)
#define WS22_CAN_ID_BUS_MEASUREMENT (WS22_CAN_BASE_STATUS + 0x02U)
#define WS22_CAN_ID_VELOCITY_MEASUREMENT (WS22_CAN_BASE_STATUS + 0x03U)
#define WS22_CAN_ID_PHASE_CURRENT (WS22_CAN_BASE_STATUS + 0x04U)
#define WS22_CAN_ID_MOTOR_VOLTAGE (WS22_CAN_BASE_STATUS + 0x05U)
#define WS22_CAN_ID_MOTOR_CURRENT (WS22_CAN_BASE_STATUS + 0x06U)
#define WS22_CAN_ID_MOTOR_BACK_EMF (WS22_CAN_BASE_STATUS + 0x07U)
#define WS22_CAN_ID_RAIL_15V (WS22_CAN_BASE_STATUS + 0x08U)
#define WS22_CAN_ID_TEMPERATURE (WS22_CAN_BASE_STATUS + 0x0BU)

/**
 * @brief Motor control command data structure
 */
typedef struct {
  float current;    /**< Motor current command (0.0 to 1.0) */
  int32_t velocity; /**< Motor velocity command (0 to 12000 rpm) */
} Ws22MotorControlData;

/**
 * @brief Motor status and telemetry data structure
 */
typedef struct {
  /* Status Information (0x401) */
  uint16_t error_flags; /**< Motor controller error flags */
  uint16_t limit_flags; /**< Motor controller limit flags */

  /* Bus Measurements (0x402) */
  float bus_current; /**< DC bus current (A) */
  float bus_voltage; /**< DC bus voltage (V) */

  /* Velocity Measurements (0x403) */
  float vehicle_velocity; /**< Vehicle velocity (m/s) */
  float motor_velocity;   /**< Motor velocity (rpm) */

  /* Phase Currents (0x404) */
  float phase_b_current; /**< Phase B current (A) */
  float phase_c_current; /**< Phase C current (A) */

  /* Motor Voltages (0x405) */
  float voltage_d; /**< D-axis voltage (V) */
  float voltage_q; /**< Q-axis voltage (V) */

  /* Motor Currents (0x406) */
  float current_d; /**< D-axis current (A) */
  float current_q; /**< Q-axis current (A) */

  /* Motor Back EMF (0x407) */
  float back_emf_d; /**< D-axis back EMF (V) */
  float back_emf_q; /**< Q-axis back EMF (V) */

  /* Power Rail (0x408) */
  float rail_15v_supply; /**< 15V rail voltage (V) */

  /* Temperature Measurements (0x40B) */
  float heat_sink_temp; /**< Heat sink temperature (°C) */
  float motor_temp;     /**< Motor temperature (°C) */
} Ws22MotorTelemetryData;

/**
 * @brief WS22 motor CAN interface storage structure
 */
typedef struct Ws22MotorCanStorage {
  Ws22MotorControlData control;     /**< Motor control data */
  Ws22MotorTelemetryData telemetry; /**< Motor telemetry data */
} Ws22MotorCanStorage;

/**
 * @brief   Initialize the WS22 motor CAN interface
 * @param   storage Pointer to Front controller storage structure
 * @return  STATUS_CODE_OK on success, error code otherwise
 */
StatusCode ws22_motor_can_init(FrontControllerStorage *storage);

/**
 * @brief   Set motor control current command
 * @param   current Motor current command (0.0 to 1.0)
 * @return  STATUS_CODE_OK on success, STATUS_CODE_INVALID_ARGS if out of range
 */
StatusCode ws22_motor_can_set_current(float current);

/**
 * @brief   Set motor control velocity command
 * @param   velocity Motor velocity command (rpm)
 * @return  STATUS_CODE_OK on success, STATUS_CODE_INVALID_ARGS if out of range
 */
StatusCode ws22_motor_can_set_velocity(int32_t velocity);

/**
 * @brief   Build and transmit motor drive command
 * @return  STATUS_CODE_OK on success, error code otherwise
 */
StatusCode ws22_motor_can_transmit_drive_command(void);

/**
 * @brief   Process received CAN message for motor controller
 * @param   msg Pointer to received CAN message
 * @return  STATUS_CODE_OK on success, STATUS_CODE_UNIMPLEMENTED for unknown message IDs
 */
StatusCode ws22_motor_can_process_rx(CanMessage *msg);

/**
 * @brief   Get current motor control data
 * @return  Pointer to motor control data (read-only)
 */
Ws22MotorControlData *ws22_motor_can_get_control_data(void);

/**
 * @brief   Get current motor telemetry data
 * @return  Pointer to motor telemetry data (read-only)
 */
Ws22MotorTelemetryData *ws22_motor_can_get_telemetry_data(void);

/** @} */
