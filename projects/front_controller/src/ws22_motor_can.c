/************************************************************************************************
 * @file   ws22_motor_can.c
 *
 * @brief  Source file for Wavesculptor 22 CAN interface
 *
 * @date   2025-06-29
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#include "log.h"

/* Intra-component Headers */
#include "ws22_motor_can.h"

static FrontControllerStorage *front_controller_storage = NULL;

static Ws22MotorCanStorage s_ws22_motor_can_storage = { 0 };

static StatusCode s_build_drive_command(Ws22MotorControlData *control_data, CanMessage *msg) {
  if (control_data == NULL || msg == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  msg->id.raw = WS22_CAN_ID_DRIVE_CMD;
  msg->extended = false;
  msg->dlc = 8U;

  /* Pack velocity (little-endian) */
  msg->data_u8[0] = (uint8_t)(control_data->velocity & 0xFFU);
  msg->data_u8[1] = (uint8_t)((control_data->velocity >> 8U) & 0xFFU);
  msg->data_u8[2] = (uint8_t)((control_data->velocity >> 16U) & 0xFFU);
  msg->data_u8[3] = (uint8_t)((control_data->velocity >> 24U) & 0xFFU);

  /* Pack current as float */
  union {
    float f;
    uint8_t bytes[4];
  } current_union;
  current_union.f = control_data->current;

  msg->data_u8[4] = current_union.bytes[0];
  msg->data_u8[5] = current_union.bytes[1];
  msg->data_u8[6] = current_union.bytes[2];
  msg->data_u8[7] = current_union.bytes[3];

  return STATUS_CODE_OK;
}

static StatusCode s_process_status_info(Ws22MotorTelemetryData *telemetry, CanMessage *msg) {
  if (telemetry == NULL || msg == NULL || msg->dlc < 4U) {
    return STATUS_CODE_INVALID_ARGS;
  }

  telemetry->limit_flags = (uint16_t)((msg->data_u8[1] << 8U) | msg->data_u8[0]);
  telemetry->error_flags = (uint16_t)((msg->data_u8[3] << 8U) | msg->data_u8[2]);

  return STATUS_CODE_OK;
}

static StatusCode s_process_bus_measurement(Ws22MotorTelemetryData *telemetry, CanMessage *msg) {
  if (telemetry == NULL || msg == NULL || msg->dlc < 8U) {
    return STATUS_CODE_INVALID_ARGS;
  }

  memcpy(&telemetry->bus_voltage, &msg->data_u8[0], sizeof(float));
  memcpy(&telemetry->bus_current, &msg->data_u8[4], sizeof(float));

  return STATUS_CODE_OK;
}

static StatusCode s_process_velocity_measurement(Ws22MotorTelemetryData *telemetry, CanMessage *msg) {
  if (telemetry == NULL || msg == NULL || msg->dlc < 8U) {
    return STATUS_CODE_INVALID_ARGS;
  }

  memcpy(&telemetry->motor_velocity, &msg->data_u8[0], sizeof(float));
  memcpy(&telemetry->vehicle_velocity, &msg->data_u8[4], sizeof(float));

  /* Read vehicle velocity is in units of m/s */
  front_controller_storage->vehicle_speed_kph = telemetry->motor_velocity * 3.6f;

  return STATUS_CODE_OK;
}

static StatusCode s_process_phase_current(Ws22MotorTelemetryData *telemetry, CanMessage *msg) {
  if (telemetry == NULL || msg == NULL || msg->dlc < 8U) {
    return STATUS_CODE_INVALID_ARGS;
  }

  memcpy(&telemetry->phase_b_current, &msg->data_u8[0], sizeof(float));
  memcpy(&telemetry->phase_c_current, &msg->data_u8[4], sizeof(float));

  return STATUS_CODE_OK;
}

static StatusCode s_process_motor_voltage(Ws22MotorTelemetryData *telemetry, CanMessage *msg) {
  if (telemetry == NULL || msg == NULL || msg->dlc < 8U) {
    return STATUS_CODE_INVALID_ARGS;
  }

  memcpy(&telemetry->voltage_q, &msg->data_u8[0], sizeof(float));
  memcpy(&telemetry->voltage_d, &msg->data_u8[4], sizeof(float));

  return STATUS_CODE_OK;
}

static StatusCode s_process_motor_current(Ws22MotorTelemetryData *telemetry, CanMessage *msg) {
  if (telemetry == NULL || msg == NULL || msg->dlc < 8U) {
    return STATUS_CODE_INVALID_ARGS;
  }

  memcpy(&telemetry->current_q, &msg->data_u8[0], sizeof(float));
  memcpy(&telemetry->current_d, &msg->data_u8[4], sizeof(float));

  return STATUS_CODE_OK;
}

static StatusCode s_process_motor_back_emf(Ws22MotorTelemetryData *telemetry, CanMessage *msg) {
  if (telemetry == NULL || msg == NULL || msg->dlc < 8U) {
    return STATUS_CODE_INVALID_ARGS;
  }

  memcpy(&telemetry->back_emf_q, &msg->data_u8[0], sizeof(float));
  memcpy(&telemetry->back_emf_d, &msg->data_u8[4], sizeof(float));

  return STATUS_CODE_OK;
}

static StatusCode s_process_rail_15v(Ws22MotorTelemetryData *telemetry, CanMessage *msg) {
  if (telemetry == NULL || msg == NULL || msg->dlc < 8U) {
    return STATUS_CODE_INVALID_ARGS;
  }

  memcpy(&telemetry->rail_15v_supply, &msg->data_u8[4], sizeof(float));

  return STATUS_CODE_OK;
}

static StatusCode s_process_temperature(Ws22MotorTelemetryData *telemetry, CanMessage *msg) {
  if (telemetry == NULL || msg == NULL || msg->dlc < 8U) {
    return STATUS_CODE_INVALID_ARGS;
  }

  memcpy(&telemetry->motor_temp, &msg->data_u8[0], sizeof(float));
  memcpy(&telemetry->heat_sink_temp, &msg->data_u8[4], sizeof(float));

  return STATUS_CODE_OK;
}

StatusCode ws22_motor_can_init(FrontControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  front_controller_storage = storage;
  front_controller_storage->ws22_motor_can_storage = &s_ws22_motor_can_storage;

  return STATUS_CODE_OK;
}

StatusCode ws22_motor_can_set_current(float current) {
  if (current < 0.0f || current > 1.0f) {
    LOG_DEBUG("Current command out of range: %f (valid: 0.0-1.0)", (double)current);
    return STATUS_CODE_INVALID_ARGS;
  }

  s_ws22_motor_can_storage.control.current = current;
  return STATUS_CODE_OK;
}

StatusCode ws22_motor_can_set_velocity(uint32_t velocity) {
  if (velocity > 12000U) {
    LOG_DEBUG("Velocity command out of range: %u (max: 12000 rpm)", velocity);
    return STATUS_CODE_INVALID_ARGS;
  }

  s_ws22_motor_can_storage.control.velocity = velocity;
  return STATUS_CODE_OK;
}

StatusCode ws22_motor_can_transmit_drive_command(void) {
  CanMessage msg;

  StatusCode status = s_build_drive_command(&s_ws22_motor_can_storage.control, &msg);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  return can_transmit(&msg);
}

StatusCode ws22_motor_can_process_rx(CanMessage *msg) {
  if (msg == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  switch (msg->id.raw) {
    case WS22_CAN_ID_STATUS_INFO:
      return s_process_status_info(&s_ws22_motor_can_storage.telemetry, msg);

    case WS22_CAN_ID_BUS_MEASUREMENT:
      return s_process_bus_measurement(&s_ws22_motor_can_storage.telemetry, msg);

    case WS22_CAN_ID_VELOCITY_MEASUREMENT:
      return s_process_velocity_measurement(&s_ws22_motor_can_storage.telemetry, msg);

    case WS22_CAN_ID_PHASE_CURRENT:
      return s_process_phase_current(&s_ws22_motor_can_storage.telemetry, msg);

    case WS22_CAN_ID_MOTOR_VOLTAGE:
      return s_process_motor_voltage(&s_ws22_motor_can_storage.telemetry, msg);

    case WS22_CAN_ID_MOTOR_CURRENT:
      return s_process_motor_current(&s_ws22_motor_can_storage.telemetry, msg);

    case WS22_CAN_ID_MOTOR_BACK_EMF:
      return s_process_motor_back_emf(&s_ws22_motor_can_storage.telemetry, msg);

    case WS22_CAN_ID_RAIL_15V:
      return s_process_rail_15v(&s_ws22_motor_can_storage.telemetry, msg);

    case WS22_CAN_ID_TEMPERATURE:
      return s_process_temperature(&s_ws22_motor_can_storage.telemetry, msg);

    default:
      return STATUS_CODE_UNIMPLEMENTED;
  }
}

Ws22MotorControlData *ws22_motor_can_get_control_data(void) {
  return &s_ws22_motor_can_storage.control;
}

Ws22MotorTelemetryData *ws22_motor_can_get_telemetry_data(void) {
  return &s_ws22_motor_can_storage.telemetry;
}
