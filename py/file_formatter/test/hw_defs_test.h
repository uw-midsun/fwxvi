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

/* Intra-component Headers */

/**
 * @defgroup bms_carrier
 * @brief    bms_carrier Firmware
 * @{
 */

/************************************************************************************************
 * BMS CAN definitions
 ************************************************************************************************/

/** @brief  BMS CAN RX Port */
#define BMS_CAN_RX \
  { .port = GPIO_PORT_B, .pin = 8 }

/** @brief  BMS CAN TX Port */
#define BMS_CAN_TX \
  { .port = GPIO_PORT_B, .pin = 9 }

/************************************************************************************************
 * Current sense I2C definitions
 ************************************************************************************************/

/** @brief  Current sense I2C port */
#define CURRENT_SENSE_I2C_PORT I2C_PORT_1

/** @brief  Current sense I2C serial data pin */
#define CURRENT_SENSE_I2C_SDA_GPIO \
  { .port = GPIO_PORT_B, .pin = 11 }

/** @brief  Current sense I2C clock pin */
#define CURRENT_SENSE_I2C_SCL_GPIO \
  { .port = GPIO_PORT_B, .pin = 10 }

/** @brief  Current sense slave address */
#define CURRENT_SENSE_MAX17261_I2C_ADDR (0x36U)

/************************************************************************************************
 * AFEs (LTC6811) SPI definitions
 ************************************************************************************************/

/** @brief  AFE SPI port */
#define LTC_AFE_SPI_PORT SPI_PORT_2

/** @brief  AFE SPI Serial data out */
#define LTC_AFE_SPI_SDO_GPIO \
  { .port = GPIO_PORT_B, .pin = 15 }

/** @brief  AFE SPI Serial data in */
#define LTC_AFE_SPI_SDI_GPIO \
  { .port = GPIO_PORT_B, .pin = 14 }

/** @brief  AFE SPI Serial clock line */
#define LTC_AFE_SPI_SCK_GPIO \
  { .port = GPIO_PORT_B, .pin = 13 }

/** @brief  AFE SPI Chip select line */
#define LTC_AFE_SPI_CS_GPIO \
  { .port = GPIO_PORT_B, .pin = 12 }

/************************************************************************************************
 * Motor Controller SPI definitions
 ************************************************************************************************/

/** @brief  MCP2515 SPI port */
#define MCP2515_SPI_PORT SPI_PORT_3

/** @brief  MCP2515 SPI Serial data out */
#define MCP2515_SPI_SDO_GPIO \
  { .port = GPIO_PORT_B, .pin = 5 }

/** @brief  MCP2515 SPI Serial data in */
#define MCP2515_SPI_SDI_GPIO \
  { .port = GPIO_PORT_B, .pin = 4 }

/** @brief  MCP2515 SPI Serial clock line */
#define MCP2515_SPI_SCK_GPIO \
  { .port = GPIO_PORT_B, .pin = 3 }

/** @brief  MCP2515 SPI Chip select line */
#define MCP2515_SPI_CS_GPIO \
  { .port = GPIO_PORT_A, .pin = 15 }

/************************************************************************************************
 * Relays/Contactor definitions
 ************************************************************************************************/

/** @brief  Precharge status pin for sensing precharge completion */
#define BMS_PRECHARGE_STATUS_GPIO \
  { .port = GPIO_PORT_A, .pin = 3 }

/** @brief  Motor relay enable pin */
#define BMS_PRECHARGE_RELAY_ENABLE_GPIO \
  { .port = GPIO_PORT_A, .pin = 9 }

/** @brief  Motor relay sense pin */
#define BMS_PRECHARGE_RELAY_SENSE_GPIO \
  { .port = GPIO_PORT_A, .pin = 10 }

/** @brief  Positive relay enable pin */
#define BMS_POS_RELAY_ENABLE_GPIO \
  { .port = GPIO_PORT_A, .pin = 4 }

/** @brief  Positive relay sense pin */
#define BMS_POS_RELAY_SENSE_GPIO \
  { .port = GPIO_PORT_A, .pin = 15 }

/** @brief  Negative relay enable pin */
#define BMS_NEG_RELAY_ENABLE_GPIO \
  { .port = GPIO_PORT_A, .pin = 13 }

/** @brief  Negtaive relay sense pin */
#define BMS_NEG_RELAY_SENSE_GPIO \
  { .port = GPIO_PORT_A, .pin = 12 }

/** @brief  Solar relay enable pin */
#define BMS_SOLAR_RELAY_ENABLE_GPIO \
  { .port = GPIO_PORT_C, .pin = 13 }

/** @brief  Solar relay sense pin */
#define BMS_SOLAR_RELAY_SENSE_GPIO \
  { .port = GPIO_PORT_A, .pin = 6 }

/************************************************************************************************
 * Killswitch definitions
 ************************************************************************************************/

/** @brief  Killswitch sense pin */
#define BMS_KILLSWITCH_SENSE_GPIO \
  { .port = GPIO_PORT_A, .pin = 11 }

/************************************************************************************************
 * Aux sense definitions
 ************************************************************************************************/

/** @brief  Aux sense ADC pin */
#define BMS_AUX_BATT_VOLTAGE_ADC \
  { .port = GPIO_PORT_A, .pin = 5 }

/************************************************************************************************
 * Fan definitions
 ************************************************************************************************/

/** @brief  Fan sense pin */
#define BMS_FAN_SENSE_1_GPIO \
  { .port = GPIO_PORT_B, .pin = 0 }

/** @brief  Fan sense pin */
#define BMS_FAN_SENSE_2_GPIO \
  { .port = GPIO_PORT_B, .pin = 1 }

/** @brief  Fan PWM control pin */
#define BMS_FAN_PWM_GPIO \
  { .port = GPIO_PORT_A, .pin = 8 }

/************************************************************************************************
 * Battery pack definitions
 ************************************************************************************************/

/** @brief  Number of modules in series */
#define NUM_SERIES_MODULES 9U

/** @brief  Number of series cells in a module */
#define NUM_SERIES_CELLS_IN_MODULE 4U

/** @brief  Number of series cells */
#define NUM_SERIES_CELLS (NUM_SERIES_CELLS_IN_MODULE * NUM_SERIES_MODULES)

/** @brief  Number of parallel cells */
#define NUM_PARALLEL_CELLS 8U

/** @brief  Cell capacity in milliamp hours */
#define CELL_CAPACITY_MAH 4850U

/** @brief  Pack capacity in milliamp hours */
#define PACK_CAPACITY_MAH (CELL_CAPACITY_MAH * NUM_PARALLEL_CELLS)

/** @} */
