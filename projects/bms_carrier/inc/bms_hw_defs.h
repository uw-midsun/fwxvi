#pragma once

/************************************************************************************************
 * @file   bms_hw_defs.h
 *
 * @brief  Header file for BMS hardware definitions
 *
 * @date   2025-01-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "gpio.h"
#include "i2c.h"

/* Intra-component Headers */

/**
 * @defgroup bms_carrier
 * @brief    bms_carrier Firmware
 * @{
 */


#define CURRENT_SENSE_I2C_PORT I2C_PORT_1
#define CURRENT_SENSE_I2C_SDA_PIN \
  { .port = GPIO_PORT_B, .pin = 11 }
#define CURRENT_SENSE_I2C_SCL_PIN \
  { .port = GPIO_PORT_B, .pin = 10 }

#define BMS_CAN_RX \
  {.port=GPIO_PORT_A, .pin=1}
#define BMS_CAN_TX\
  {.port=GPIO_PORT_A, .pin=2}

#define BMS_AFE_I2C_PORT I2C_PORT_2
#define BMS_AFE_I@C\
{.port=GPIO}
  #define MAX17261_I2C_ADDR (0x36)
  
  #define LTC_AFE_I2C_ADDR (0x30)
  #define LTC_AFE_I2C_PORT BMS_I2C_PORT
  #define LTC_AFE_ALERT_PIN \
    { .port = GPIO_PORT_A, .pin = 8 }

  #define BMS_PRECHARGE_RELAY_GPIO \
    { .port = GPIO_PORT_B, .pin = 7 }
  #define BMS_PRECHARGE_STATUS_GPIO \
    { .port = GPIO_PORT_B, .pin = 6 }
  
  #define BMS_MAIN_CONTACTOR_GPIO \
    { .port = GPIO_PORT_C, .pin = 1 }
  #define BMS_MAIN_CONTACTOR_FEEDBACK_GPIO \
    { .port = GPIO_PORT_C, .pin = 2 }
  
  #define BMS_ADC_CELL_VOLTAGE_GPIO \
    { .port = GPIO_PORT_A, .pin = 1 }
  #define BMS_ADC_CELL_TEMP_GPIO \
    { .port = GPIO_PORT_A, .pin = 2 }
  
  #define BMS_AUX_BATT_VOLTAGE_GPIO \
    { .port = GPIO_PORT_B, .pin = 5 }
  
  #define NUM_SERIES_CELLS (4 * 9)
  #define NUM_PARALLEL_CELLS (8)
  #define CELL_CAPACITY_MAH 4850
  #define PACK_CAPACITY_MAH (CELL_CAPACITY_MAH * NUM_PARALLEL_CELLS)
  
  #define BMS_FAULT_OVERVOLTAGE 0x01
  #define BMS_FAULT_UNDERVOLTAGE 0x02
  #define BMS_FAULT_OVERTEMP 0x04
  #define BMS_FAULT_UNBALANCE 0x08
  #define BMS_FAULT_OVERCURRENT 0x10
  #define BMS_FAULT_RELAY_ERROR 0x20
  #define BMS_FAULT_COMMS_ERROR 0x40
  #define BMS_FAULT_KILLSWITCH 0x80
  
  #define BMS_STATUS_LED \
    { .port = GPIO_PORT_C, .pin = 3 }
  
  /** @} */
  