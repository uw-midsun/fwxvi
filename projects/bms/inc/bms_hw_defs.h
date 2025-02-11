#pragma once
#include "gpio.h"
#include "i2c.h"

#define BMS_IO_EXP_ADDR 0x20
#define BMS_I2C_PORT I2C_PORT_2
#define BMS_I2C_SDA_PIN \
  { .port = GPIO_PORT_B, .pin = 11 }
#define BMS_I2C_SCL_PIN \
  { .port = GPIO_PORT_B, .pin = 10 }

  #define MAX17261_I2C_ADDR (0x36)
  #define MAX17261_I2C_PORT BMS_I2C_PORT
  

  #define BMS_SPI_MOSI \
  { .port = GPIO_PORT_B, .pin = 15 }
#define BMS_SPI_MISO \
  { .port = GPIO_PORT_B, .pin = 14 }
#define BMS_SPI_SCK \
  { .port = GPIO_PORT_B, .pin = 13 }
#define BMS_SPI_NSS \
  { .port = GPIO_PORT_A, .pin = 15 }

#define BMS_PRECHARGE_RELAY \
  { .port = GPIO_PORT_A, .pin = 3 }
#define BMS_PRECHARGE_STATUS \
  { .port = GPIO_PORT_A, .pin = 4 }

#define BMS_MAIN_CONTACTOR \
  { .port = GPIO_PORT_A, .pin = 10 }
#define BMS_MAIN_CONTACTOR_FEEDBACK \
  { .port = GPIO_PORT_A, .pin = 11 }

#define BMS_CELL_VOLTAGE_ADC \
  { .port = GPIO_PORT_A, .pin = 2 }
#define BMS_CELL_TEMP_ADC \
  { .port = GPIO_PORT_A, .pin = 1 }

  #define BMS_AUX_BATT_VOLTAGE \
  { .port = GPIO_PORT_B, .pin = 5 }

  #define BMS_FAN_SESNE_1\{
    .port=GPIO_PORT_B, .pin=0
  }
  #define BMS_FAN_SENSE_2\ {
    .port=GPIO_PORT_B, .pin=1
  }
  #define BMS_FAN_PWM{
    .port=GPIO_PORT_A, .pin=8
  }

  #define BMS_KILLSWITCH_MNTR\{
    .port=GPIO_PORT_A, .pin=11
  }
  #define BMS_HV_POS_RELAY_SENSE \
  { .port = GPIO_PORT_A, .pin = 5 }
#define BMS_HV_NEG_RELAY_SENSE \
  { .port = GPIO_PORT_A, .pin = 14 }
#define BMS_HV_SOLAR_RELAY_SENSE \
  { .port = GPIO_PORT_A, .pin = 6 }
  
  
  #define BMS_PRECHARGE_RELAY_PIN { 
    .port = GPIO_PORT_B, .pin = 7 }
#define BMS_PRECHARGE_STATUS_PIN { 
  .port = GPIO_PORT_B, .pin = 6 }
#define BMS_HV_SOLAR_RELAY_EN { 
  .port = GPIO_PORT_C, .pin = 13 }

  #define CELL_CAPACITY_MAH 4850
  #define NUM_SERIES_CELLS (4 * 9)
  #define NUM_PARALLEL_CELLS (8)
  #define PACK_CAPACITY_MAH (CELL_CAPACITY_MAH * NUM_PARALLEL_CELLS)
  
  #define BMS_FAULT_OVERVOLTAGE,
  #define BMS_FAULT_UNBALANCE,
  #define BMS_FAULT_OVERTEMP_AMBIENT,
  #define BMS_FAULT_COMMS_LOSS_AFE,
  #define BMS_FAULT_COMMS_LOSS_CURR_SENSE,
  #define BMS_FAULT_OVERTEMP_CELL,
  #define BMS_FAULT_OVERCURRENT,
  #define BMS_FAULT_UNDERVOLTAGE,
  #define BMS_FAULT_KILLSWITCH,
  #define BMS_FAULT_RELAY_CLOSE_FAILED,
  #define BMS_FAULT_DISCONNECTED

  
  #endif // BMS_HW_DEFS_H
  