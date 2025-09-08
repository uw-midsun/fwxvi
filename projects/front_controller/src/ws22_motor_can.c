/************************************************************************************************
 * @file    ws22_motor_can.c
 *
 * @brief   Source file for Wavesculptor 22 CAN
 *
 * @date    2025-06-29
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "ws22_motor_can.h"

TxData tx_data;
RxData rx_data;

CanMessage msg;

// Setter functions
void tx_set_current(float current) {
  tx_data.current = current;
}

void tx_set_velocity(uint32_t velocity) {
  tx_data.velocity = velocity;
}

CanMessage build_drive_command(CanId id) {
  msg.id.raw = MOTOR_CAN_CONTROL_BASE + 0x01;
  msg.extended = false;
  msg.dlc = 8;

  // Velocity
  msg.data_u8[0] = tx_data.velocity;
  msg.data_u8[1] = tx_data.velocity >> 8;
  msg.data_u8[2] = tx_data.velocity >> 16;
  msg.data_u8[3] = tx_data.velocity >> 24;

  // Convert float to 4 bytes
  union {
    float f;
    uint8_t bytes[4];
  } current_union;
  current_union.f = tx_data.current;

  // Current
  msg.data_u8[4] = current_union.bytes[0];
  msg.data_u8[5] = current_union.bytes[1];
  msg.data_u8[6] = current_union.bytes[2];
  msg.data_u8[7] = current_union.bytes[3];

  return msg;
}

StatusCode motor_can_transmit(uint32_t id, bool extended, const uint8_t *msg, uint8_t dlc) {
  if (msg == NULL || dlc > 8) {
    return STATUS_CODE_INVALID_ARGS;
  }

  CanMessage tx_msg;
  tx_msg.id.raw = id;
  tx_msg.extended = extended;
  tx_msg.dlc = dlc;

  for (uint8_t i = 0; i < dlc; i++) {
    tx_msg.data_u8[i] = msg[i];
  }
  for (uint8_t i = dlc; i < 8; i++) {
    tx_msg.data_u8[i] = 0;
  }

  return can_transmit(&tx_msg);
}

// rx data setter functions
void rx_set_limit_flags(uint16_t flags) {
  rx_data.limit_flags = flags;
}
void rx_set_error_flags(uint16_t flags) {
  rx_data.error_flags = flags;
}
void rx_set_bus_current(float current) {
  rx_data.bus_current = current;
}
void rx_set_bus_voltage(float voltage) {
  rx_data.bus_voltage = voltage;
}
void rx_set_vehicle_velocity(float velocity) {
  rx_data.vehicle_velocity = velocity;
}
void rx_set_motor_velocity(float velocity) {
  rx_data.motor_velocity = velocity;
}
void rx_set_phase_c_current(float current) {
  rx_data.phase_c_current = current;
}
void rx_set_phase_b_current(float current) {
  rx_data.phase_b_current = current;
}
void rx_set_voltage_d(float voltage) {
  rx_data.voltage_d = voltage;
}
void rx_set_voltage_q(float voltage) {
  rx_data.voltage_q = voltage;
}
void rx_set_current_d(float current) {
  rx_data.current_d = current;
}
void rx_set_current_q(float current) {
  rx_data.current_q = current;
}
void rx_set_back_EMF_d(float voltage) {
  rx_data.back_EMF_d = voltage;
}
void rx_set_back_EMF_q(float voltage) {
  rx_data.back_EMF_q = voltage;
}
void rx_set_rail_15v_supply(float voltage) {
  rx_data.rail_15V_supply = voltage;
}
void rx_set_heat_sink_temp(float degrees) {
  rx_data.heat_sink_temp = degrees;
}
void rx_set_motor_temp(float degrees) {
  rx_data.motor_temp = degrees;
}

// filling rx_data struct
StatusCode motor_can_rx_all_cb(CanMessage *msg) {
  switch (msg->id.raw) {
    case STATUS_INFO: {
      uint16_t limit_flags = (msg->data_u8[1] << 8) | msg->data_u8[0];
      uint16_t error_flags = (msg->data_u8[3] << 8) | msg->data_u8[2];

      rx_set_limit_flags(limit_flags);
      rx_set_error_flags(error_flags);
      break;
    }

    case BUS_MEASUREMENT: {
      float voltage, current;
      memcpy(&voltage, &msg->data_u8[0], sizeof(float));
      memcpy(&current, &msg->data_u8[4], sizeof(float));

      rx_set_bus_current(current);
      rx_set_bus_voltage(voltage);
      break;
    }
    case VELOCTIY_MEASUREMENT: {
      float motor, vehicle;
      memcpy(&motor, &msg->data_u8[0], sizeof(float));
      memcpy(&vehicle, &msg->data_u8[4], sizeof(float));

      rx_set_vehicle_velocity(vehicle);
      rx_set_motor_velocity(motor);
      break;
    }
    case PHASE_CURRENT: {
      float b, c;
      memcpy(&b, &msg->data_u8[0], sizeof(float));
      memcpy(&c, &msg->data_u8[4], sizeof(float));

      rx_set_phase_c_current(c);
      rx_set_phase_b_current(b);
      break;
    }
    case MOTOR_VOLTAGE: {
      float d, q;
      memcpy(&q, &msg->data_u8[0], sizeof(float));
      memcpy(&d, &msg->data_u8[4], sizeof(float));

      rx_set_voltage_d(d);
      rx_set_voltage_q(q);
      break;
    }
    case MOTOR_CURRENT: {
      float d, q;
      memcpy(&q, &msg->data_u8[0], sizeof(float));
      memcpy(&d, &msg->data_u8[4], sizeof(float));

      rx_set_current_d(d);
      rx_set_current_q(q);

      break;
    }
    case MOTOR_BACK_EMF: {
      float d, q;
      memcpy(&q, &msg->data_u8[0], sizeof(float));
      memcpy(&d, &msg->data_u8[4], sizeof(float));

      rx_set_back_EMF_d(d);
      rx_set_back_EMF_q(q);
      break;
    }
    case RAIL_15V: {
      float voltage;
      memcpy(&voltage, &msg->data_u8[4], sizeof(float));

      rx_set_rail_15v_supply(voltage);
      break;
    }
    case HEAT_SINK_MOTOR_TEMP: {
      float heat_sink, motor;
      memcpy(&motor, &msg->data_u8[0], sizeof(float));
      memcpy(&heat_sink, &msg->data_u8[4], sizeof(float));

      rx_set_heat_sink_temp(heat_sink);
      rx_set_motor_temp(motor);
      break;
    }
  }
  return STATUS_CODE_OK;
}

RxData get_rx_data(void) {
  return rx_data;
}
