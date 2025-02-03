#pragma once

/************************************************************************************************
 * @file   pd_hw_defs.h
 *
 * @brief  Header file for power distribution hardware definitions
 *
 * @date   2025-01-31
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "gpio.h"
#include "i2c.h"

/* Intra-component Headers */

/**
 * @defgroup power_distribution
 * @brief    power_distribution Firmware
 * @{
 */

/************************************************************************************************
 * IO EXP (PCA9555) I2C definitions???
 ************************************************************************************************/
#define PD_I2C_PORT I2C_PORT_2

#define PD_PCA9555_I2C_ADDR0 0x20 
#define PD_PCA9555_I2C_ADDR1 0x21

#define PD_I2C_SDA_PIN \
  { .port = GPIO_PORT_B, .pin = 11 }
#define PD_I2C_SCL_PIN \
  { .port = GPIO_PORT_B, .pin = 10 }

#define PD_PCA9555_INT0_PIN \
  { .port = GPIO_PORT_B, .pin = 0 }
#define PD_PCA9555_INT1_PIN \
  { .port = GPIO_PORT_B, .pin = 1 }

/************************************************************************************************
 * IO EXP Pin Definitions
 ************************************************************************************************/
#define RELAY_EN \
  { .port = PD_PCA9555_I2C_ADDR0, .pin = PCA9555_PIN_IO0_0 }
#define RELAY_FAN_DS \
  { .port = PD_PCA9555_I2C_ADDR0, .pin = PCA9555_PIN_IO0_1 }
#define DRIVER_FAN_EN \
  { .port = PD_PCA9555_I2C_ADDR0, .pin = PCA9555_PIN_IO0_2 }
#define TELEMETRY_EN \
  { .port = PD_PCA9555_I2C_ADDR0, .pin = PCA9555_PIN_IO0_3 }
#define TELEMENTRY_DS \
  { .port = PD_PCA9555_I2C_ADDR0, .pin = PCA9555_PIN_IO0_4 }
#define SPARE_5V_AUX_EN \
  { .port = PD_PCA9555_I2C_ADDR0, .pin = PCA9555_PIN_IO0_5}
#define CAMERA_EN \
  { .port = PD_PCA9555_I2C_ADDR0, .pin = PCA9555_PIN_IO0_6 }
#define CAM_DS \
  { .port = PD_PCA9555_I2C_ADDR0, .pin = PCA9555_PIN_IO0_7 }
#define SPARE_5V_DCDC_EN \
  { .port = PD_PCA9555_I2C_ADDR0, .pin = PCA9555_PIN_IO0_10 }
#define SPARE1_12V_DCDC_EN \
  { .port = PD_PCA9555_I2C_ADDR0, .pin = PCA9555_PIN_IO0_11 }
#define SPARE_DCDC_DS \
  { .port = PD_PCA9555_I2C_ADDR0, .pin = PCA9555_PIN_IO0_12 }
#define SPARE2_12V_DCDC_EN \
  { .port = PD_PCA9555_I2C_ADDR0, .pin = PCA9555_PIN_IO0_13 }
#define MCI_EN \
  { .port = PD_PCA9555_I2C_ADDR0, .pin = PCA9555_PIN_IO0_14 }
#define MCI_BPS_DS \
  { .port = PD_PCA9555_I2C_ADDR0, .pin = PCA9555_PIN_IO0_15 }
#define BPS_LIGHT_EN \
  { .port = PD_PCA9555_I2C_ADDR0, .pin = PCA9555_PIN_IO0_16 }
  
#define LR_TURN_DS \
  { .port = PD_PCA9555_I2C_ADDR1, .pin = PCA9555_PIN_IO0_0 }
#define RIGHT_TURN_EN \
  { .port = PD_PCA9555_I2C_ADDR1, .pin = PCA9555_PIN_IO0_1 }
#define PEDAL_EN \
  { .port = PD_PCA9555_I2C_ADDR1, .pin = PCA9555_PIN_IO0_2 }
#define PEDAL_STEERING_DS \
  { .port = PD_PCA9555_I2C_ADDR1, .pin = PCA9555_PIN_IO0_3 }
#define STEERING_EN \
  { .port = PD_PCA9555_I2C_ADDR1, .pin = PCA9555_PIN_IO0_4 }
#define SOLAR1_EN \
  { .port = PD_PCA9555_I2C_ADDR1, .pin = PCA9555_PIN_IO0_5}
#define SOLAR_DS \
  { .port = PD_PCA9555_I2C_ADDR1, .pin = PCA9555_PIN_IO0_6 }
#define SOLAR2_EN \
  { .port = PD_PCA9555_I2C_ADDR1, .pin = PCA9555_PIN_IO0_7 }
#define LEFT_TURN_EN \
  { .port = PD_PCA9555_I2C_ADDR1, .pin = PCA9555_PIN_IO0_10 }
#define CONSOLE_EN \
  { .port = PD_PCA9555_I2C_ADDR1, .pin = PCA9555_PIN_IO0_11 }
#define CC_BMS_DS \
  { .port = PD_PCA9555_I2C_ADDR1, .pin = PCA9555_PIN_IO0_12 }
#define BMS_EN \
  { .port = PD_PCA9555_I2C_ADDR1, .pin = PCA9555_PIN_IO0_13 }
#define DRL_EN \
  { .port = PD_PCA9555_I2C_ADDR1, .pin = PCA9555_PIN_IO0_14 }
#define DRL_BRAKE_DS \
  { .port = PD_PCA9555_I2C_ADDR1, .pin = PCA9555_PIN_IO0_15 }
#define BRAKE_LIGHT_EN \
  { .port = PD_PCA9555_I2C_ADDR1, .pin = PCA9555_PIN_IO0_16 }

/************************************************************************************************
 * Current Sense Mux definitions
 ************************************************************************************************/
#define NUM_MUX_SEL_PINS 4
#define NUM_MUX_INPUTS (1 << NUM_MUX_SEL_PINS) //0b10000 -> 16 inputs

#define PD_MUX_OUTPUT_PIN \
  { .port = GPIO_PORT_2, .pin = 6 }
#define PD_MUX_SEL4_PIN \
  { .port = GPIO_PORT_A, .pin = 14 }
#define PD_MUX_SEL3_PIN \
  { .port = GPIO_PORT_A, .pin = 13 }
#define PD_MUX_SEL2_PIN \
  { .port = GPIO_PORT_A, .pin = 12 }
#define PD_MUX_SEL1_PIN \
  { .port = GPIO_PORT_A, .pin = 11 }

#define MUX_SEL_PEDAL_STEERING 6
#define MUX_SEL_SOLAR 7
#define MUX_SEL_LR_TURN 8
#define MUX_SEL_DRL_BRAKE 9
#define MUX_SEL_CC_BMS 10
#define MUX_SEL_RELAY_FAN 11
#define MUX_SEL_TELEMTRY 12
#define MUX_SEL_CAM 13
#define MUX_SEL_DSPARE 14
#define MUX_SEL_MCI_BPS 15

/************************************************************************************************
 * Voltage and Current Sense Definitions
 ************************************************************************************************/
#define AUX_VSENSE_PIN \
  { .port = GPIO_PORT_A, .pin = 2 }
#define AUX_ISENSE_PIN \
  { .port = GPIO_PORT_A, .pin = 3 }

#define DCDC_VSENSE_PIN \
  { .port = GPIO_PORT_A, .pin = 4 }
#define DCDC_ISENSE_PIN \
  { .port = GPIO_PORT_A, .pin = 5 }

/************************************************************************************************
 * Fault Pin Definitions (AUX, DCDC)
 ************************************************************************************************/
#define AUX_VALID1_PIN \
  { .port = GPIO_PORT_B, .pin = 5 }
#define AUX_VALID2_PIN \
  { .port = GPIO_PORT_B, .pin = 4 }

#define DCDC_VALID1_PIN \
  { .port = GPIO_PORT_B, .pin = 3 }
#define DCDC_VALID2_PIN \
  { .port = GPIO_PORT_A, .pin = 15 }
