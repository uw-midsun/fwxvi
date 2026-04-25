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

static Ws22MotorCanStorage *s_ws22_motor_can_storage = NULL;
static Ws22MotorCanConfig *s_ws22_motor_can_config = NULL;

static StatusCode s_process_status_info(Ws22MotorTelemetryData *telemetry, uint8_t *msg_data_u8, uint8_t msg_dlc) {
  if (telemetry == NULL || msg_data_u8 == NULL || msg_dlc < 4U) {
    return STATUS_CODE_INVALID_ARGS;
  }

  telemetry->limit_flags = (uint16_t)((msg_data_u8[1] << 8U) | msg_data_u8[0]);
  telemetry->error_flags = (uint16_t)((msg_data_u8[3] << 8U) | msg_data_u8[2]);

  return STATUS_CODE_OK;
}

static StatusCode s_process_bus_measurement(Ws22MotorTelemetryData *telemetry, uint8_t *msg_data_u8, uint8_t msg_dlc) {
  if (telemetry == NULL || msg_data_u8 == NULL || msg_dlc < 8U) {
    return STATUS_CODE_INVALID_ARGS;
  }

  memcpy(&telemetry->bus_voltage, &msg_data_u8[0], sizeof(float));
  memcpy(&telemetry->bus_current, &msg_data_u8[4], sizeof(float));
  // LOG_DEBUG("Busvolt reading %d\r\n", (int16_t)telemetry->bus_voltage);

  return STATUS_CODE_OK;
}

static StatusCode s_process_velocity_measurement(Ws22MotorTelemetryData *telemetry, uint8_t *msg_data_u8, uint8_t msg_dlc) {
  if (telemetry == NULL || msg_data_u8 == NULL || msg_dlc < 8U) {
    return STATUS_CODE_INVALID_ARGS;
  }
  LOG_DEBUG("data: %x %x %x %x\r\n", msg_data_u8[0], msg_data_u8[1], msg_data_u8[2], msg_data_u8[3]);

  memcpy(&telemetry->motor_velocity, &msg_data_u8[0], sizeof(float));
  memcpy(&telemetry->vehicle_velocity, &msg_data_u8[4], sizeof(float));

  /* Read vehicle velocity is in units of m/s */
  //   front_controller_storage->vehicle_speed_kph = telemetry->motor_velocity * 3.6f;

  return STATUS_CODE_OK;
}

static StatusCode s_process_phase_current(Ws22MotorTelemetryData *telemetry, uint8_t *msg_data_u8, uint8_t msg_dlc) {
  if (telemetry == NULL || msg_data_u8 == NULL || msg_dlc < 8U) {
    return STATUS_CODE_INVALID_ARGS;
  }

  memcpy(&telemetry->phase_b_current, &msg_data_u8[0], sizeof(float));
  memcpy(&telemetry->phase_c_current, &msg_data_u8[4], sizeof(float));

  return STATUS_CODE_OK;
}

static StatusCode s_process_motor_voltage(Ws22MotorTelemetryData *telemetry, uint8_t *msg_data_u8, uint8_t msg_dlc) {
  if (telemetry == NULL || msg_data_u8 == NULL || msg_dlc < 8U) {
    return STATUS_CODE_INVALID_ARGS;
  }

  memcpy(&telemetry->voltage_q, &msg_data_u8[0], sizeof(float));
  memcpy(&telemetry->voltage_d, &msg_data_u8[4], sizeof(float));

  return STATUS_CODE_OK;
}

static StatusCode s_process_motor_current(Ws22MotorTelemetryData *telemetry, uint8_t *msg_data_u8, uint8_t msg_dlc) {
  if (telemetry == NULL || msg_data_u8 == NULL || msg_dlc < 8U) {
    return STATUS_CODE_INVALID_ARGS;
  }

  memcpy(&telemetry->current_q, &msg_data_u8[0], sizeof(float));
  memcpy(&telemetry->current_d, &msg_data_u8[4], sizeof(float));

  return STATUS_CODE_OK;
}

static StatusCode s_process_motor_back_emf(Ws22MotorTelemetryData *telemetry, uint8_t *msg_data_u8, uint8_t msg_dlc) {
  if (telemetry == NULL || msg_data_u8 == NULL || msg_dlc < 8U) {
    return STATUS_CODE_INVALID_ARGS;
  }

  memcpy(&telemetry->back_emf_q, &msg_data_u8[0], sizeof(float));
  memcpy(&telemetry->back_emf_d, &msg_data_u8[4], sizeof(float));

  return STATUS_CODE_OK;
}

static StatusCode s_process_rail_15v(Ws22MotorTelemetryData *telemetry, uint8_t *msg_data_u8, uint8_t msg_dlc) {
  if (telemetry == NULL || msg_data_u8 == NULL || msg_dlc < 8U) {
    return STATUS_CODE_INVALID_ARGS;
  }

  memcpy(&telemetry->rail_15v_supply, &msg_data_u8[4], sizeof(float));

  return STATUS_CODE_OK;
}

static StatusCode s_process_temperature(Ws22MotorTelemetryData *telemetry, uint8_t *msg_data_u8, uint8_t msg_dlc) {
  if (telemetry == NULL || msg_data_u8 == NULL || msg_dlc < 8U) {
    return STATUS_CODE_INVALID_ARGS;
  }

  memcpy(&telemetry->motor_temp, &msg_data_u8[0], sizeof(float));
  memcpy(&telemetry->heat_sink_temp, &msg_data_u8[4], sizeof(float));

  return STATUS_CODE_OK;
}

StatusCode ws22_motor_can_init(Ws22MotorCanStorage *storage, Ws22MotorCanConfig *config) {
  if (storage == NULL || config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_ws22_motor_can_storage = storage;
  s_ws22_motor_can_config = config;

  return STATUS_CODE_OK;
}

StatusCode ws22_motor_can_set_current(float current) {
  if (current < 0.0f || current > 1.0f) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_ws22_motor_can_storage->control.current = current;
  return STATUS_CODE_OK;
}

StatusCode ws22_motor_can_set_velocity(float velocity) {
  if (velocity > 12000 || velocity < -12000) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_ws22_motor_can_storage->control.velocity = velocity;
  return STATUS_CODE_OK;
}

StatusCode ws22_motor_can_process_rx(uint8_t *msg_data_u8, uint32_t msg_id_raw, uint8_t msg_dlc) {
  if (msg_data_u8 == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  switch (msg_id_raw) {
    case WS22_CAN_ID_STATUS_INFO:
      if (!s_ws22_motor_can_config->ws22_status_info_enabled) {
        return STATUS_CODE_OK;
      }
      return s_process_status_info(&s_ws22_motor_can_storage->telemetry, msg_data_u8, msg_dlc);

    case WS22_CAN_ID_BUS_MEASUREMENT:
      if (!s_ws22_motor_can_config->ws22_bus_measurement_enabled) {
        return STATUS_CODE_OK;
      }
      return s_process_bus_measurement(&s_ws22_motor_can_storage->telemetry, msg_data_u8, msg_dlc);

    case WS22_CAN_ID_VELOCITY_MEASUREMENT:
      if (!s_ws22_motor_can_config->ws22_velocity_measurement_enabled) {
        return STATUS_CODE_OK;
      }
      return s_process_velocity_measurement(&s_ws22_motor_can_storage->telemetry, msg_data_u8, msg_dlc);

    case WS22_CAN_ID_PHASE_CURRENT:
      if (!s_ws22_motor_can_config->ws22_phase_current_enabled) {
        return STATUS_CODE_OK;
      }
      return s_process_phase_current(&s_ws22_motor_can_storage->telemetry, msg_data_u8, msg_dlc);

    case WS22_CAN_ID_MOTOR_VOLTAGE:
      if (!s_ws22_motor_can_config->ws22_motor_voltage_enabled) {
        return STATUS_CODE_OK;
      }
      return s_process_motor_voltage(&s_ws22_motor_can_storage->telemetry, msg_data_u8, msg_dlc);

    case WS22_CAN_ID_MOTOR_CURRENT:
      if (!s_ws22_motor_can_config->ws22_motor_current_enabled) {
        return STATUS_CODE_OK;
      }
      return s_process_motor_current(&s_ws22_motor_can_storage->telemetry, msg_data_u8, msg_dlc);

    case WS22_CAN_ID_MOTOR_BACK_EMF:
      if (!s_ws22_motor_can_config->ws22_motor_back_emf_enabled) {
        return STATUS_CODE_OK;
      }
      return s_process_motor_back_emf(&s_ws22_motor_can_storage->telemetry, msg_data_u8, msg_dlc);

    case WS22_CAN_ID_RAIL_15V:
      if (!s_ws22_motor_can_config->ws22_rail_15v_enabled) {
        return STATUS_CODE_OK;
      }
      return s_process_rail_15v(&s_ws22_motor_can_storage->telemetry, msg_data_u8, msg_dlc);

    case WS22_CAN_ID_TEMPERATURE:
      if (!s_ws22_motor_can_config->ws22_temperature_enabled) {
        return STATUS_CODE_OK;
      }
      return s_process_temperature(&s_ws22_motor_can_storage->telemetry, msg_data_u8, msg_dlc);

    default:
      return STATUS_CODE_UNIMPLEMENTED;
  }
}

Ws22MotorControlData *ws22_motor_can_get_control_data(void) {
  return &s_ws22_motor_can_storage->control;
}

Ws22MotorTelemetryData *ws22_motor_can_get_telemetry_data(void) {
  return &s_ws22_motor_can_storage->telemetry;
}
