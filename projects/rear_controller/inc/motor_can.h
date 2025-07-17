#pragma once

/************************************************************************************************
 * @file   rear_controller.h
 *
 * @brief  Header file for motor_can.c
 *
 * @date   2025-06-29
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "can_hw.h"
#include "can_msg.h"
#include "can.h"

/* Intra-component Headers */

/**
 * @defgroup Rear Controller
 * @brief    Rear Controller Board Firmware
 * @{
 */

#define MOTOR_CAN_CONTROL_BASE 0x500
#define MOTOR_CAN_CONTROLLER_BASE 0x400

#define STATUS_INFO (MOTOR_CAN_CONTROLLER_BASE + 0X01)
#define BUS_MEASUREMENT (MOTOR_CAN_CONTROLLER_BASE + 0X02)
#define VELOCTIY_MEASUREMENT (MOTOR_CAN_CONTROLLER_BASE + 0X03)
#define PHASE_CURRENT (MOTOR_CAN_CONTROLLER_BASE + 0X04)
#define MOTOR_VOLTAGE (MOTOR_CAN_CONTROLLER_BASE + 0X05)
#define MOTOR_CURRENT (MOTOR_CAN_CONTROLLER_BASE + 0X06)
#define MOTOR_BACK_EMF (MOTOR_CAN_CONTROLLER_BASE + 0x07)
#define RAIL_15V (MOTOR_CAN_CONTROLLER_BASE + 0x08)
#define HEAT_SINK_MOTOR_TEMP (MOTOR_CAN_CONTROLLER_BASE + 0x0B)


typedef struct TxData {
    float current;  // Range: 0.0 to 1.0
    uint32_t velocity;   // Range: 0 to ~12000 rpm
} TxData;

typedef struct RxData{

    // Status Info - base address + 0x01
    uint16_t error_flags;
    uint16_t limit_flags;

    //Bus Measurement - base address + 0x02
    float bus_current;
    float bus_voltage;

    // Velocity Measurement - base address + 0x03
    float vehicle_velocity;
    float motor_velocity;

    // Phase Current - base address + 0x04
    float phase_c_current;
    float phase_b_current;

    // Motor Voltage - base address + 0x05
    float voltage_d;
    float voltage_q;

    //Motor Current - base address + 0x06
    float current_d;
    float current_q;

    // Motor BackEMF  - base address + 0x07
    float back_EMF_d; //always zero
    float back_EMF_q;

    // 15V Voltage Rail Measurement - base address + 0x08
    float rail_15V_supply;

    // Heat-sink and Motor Temp - base address + 0x0B
    float heat_sink_temp;
    float motor_temp;

} RxData;

StatusCode motor_can_transmit(uint32_t id, bool extended, const uint8_t *msg, uint8_t dlc);

void tx_set_current(float current);
void tx_set_velocity(uint32_t velocity);

RxData get_rx_data(void);

void rx_set_limit_flags(uint16_t flags);
void rx_set_error_flags(uint16_t flags);
void rx_set_bus_current(float current);
void rx_set_bus_voltage(float voltage);
void rx_set_vehicle_velocity(float velocity);
void rx_set_motor_velocity(float velocity);
void rx_set_phase_c_current(float current);
void rx_set_phase_b_current(float current);
void rx_set_voltage_d(float voltage);
void rx_set_voltage_q(float voltage);
void rx_set_current_d(float current);
void rx_set_current_q(float current);
void rx_set_back_EMF_d(float voltage);
void rx_set_back_EMF_q(float voltage);
void rx_set_rail_15v_supply(float voltage);
void rx_set_heat_sink_temp(float degrees);
void rx_set_motor_temp(float degrees);


/** @} */
