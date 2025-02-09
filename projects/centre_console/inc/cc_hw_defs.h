#pragma once

/************************************************************************************************
 * @file   cc_hw_defs.h
 *
 * @brief  Header file for Centre Console hardware definitions
 *
 * @date   2025-01-30
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "gpio.h"
#include "i2c.h"

/* Intra-component Headers */

/**
 * @defgroup centre_console
 * @brief    centre_console Firmware
 * @{
 */

// I/O Expander
#define CC_IO_EXP_ADDR 0x20
#define CC_IO_EXP_I2C_PORTS 2
#define CC_IO_EXP_SDA \
  { .port = GPIO_PORT_A, .pin = 10 }
#define CC_IO_EXP_SCL \
  { .port = GPIO_PORT_A, .pin = 9 }

// Individual Component Defines
#define ADC_HALL_SENSOR \
  { .port = GPIO_PORT_A, .pin = 0 }
#define LED_BPS \
  { .port = GPIO_PORT_A, .pin = 8 }
#define ADC_TURN_SIGNAL \
  { .port = GPIO_PORT_A, .pin = 6 }
#define ADC_CRUISE_CONTROL \
  { .port = GPIO_PORT_A, .pin = 7 }
#define BUZZER \
  { .port = GPIO_PORT_B, .pin = 15 }
#define BRAKE_LIMIT_SWITCH \
  { .port = GPIO_PORT_B, .pin = 13 }
#define CRUISE_CONTROL_BUTTON \
  { .port = GPIO_PORT_B, .pin = 12 }

// Buttons
#define BUTTONS_SDA \
  { .port = GPIO_PORT_A, .pin = 10 }
#define BUTTONS_SCL \
  { .port = GPIO_PORT_A, .pin = 9 }

// Speedometer
#define SPEED_DISP1 \
  { .port = GPIO_PORT_A, .pin = 1 }
#define SPEED_DISP2 \
  { .port = GPIO_PORT_A, .pin = 4 }
#define SPEED_DISP3 \
  { .port = GPIO_PORT_A, .pin = 3 }
#define SPEED_DISP4 \
  { .port = GPIO_PORT_A, .pin = 2 }

// Batter Current Display
#define BATT_CURR_DISP1 \
  { .port = GPIO_PORT_A, .pin = 5 }
#define BATT_CURR_DISP2 \
  { .port = GPIO_PORT_B, .pin = 0 }
#define BATT_CURR_DISP3 \
  { .port = GPIO_PORT_B, .pin = 7 }
#define BATT_CURR_DISP4 \
  { .port = GPIO_PORT_B, .pin = 6 }

// Battery Voltage Display
#define BATT_VOLT_DISP1 \
  { .port = GPIO_PORT_B, .pin = 1 }
#define BATT_VOLT_DISP2 \
  { .port = GPIO_PORT_B, .pin = 11 }
#define BATT_VOLT_DISP3 \
  { .port = GPIO_PORT_B, .pin = 10 }
#define BATT_VOLT_DISP4 \
  { .port = GPIO_PORT_B, .pin = 2 }

// All Displays
#define ALL_DISPLAYS                                                                            \
  {                                                                                             \
    .A1 = SPEED_DISP1, .B1 = SPEED_DISP2, .C1 = SPEED_DISP3, .D1 = SPEED_DISP4,                 \
    .A2 = BATT_CURR_DISP1, .B2 = BATT_CURR_DISP2, .C2 = BATT_CURR_DISP3, .D2 = BATT_CURR_DISP4, \
    .A3 = BATT_VOLT_DISP1, .B3 = BATT_VOLT_DISP2, .C3 = BATT_VOLT_DISP3, .D3 = BATT_VOLT_DISP4  \
  }

/** @} */
