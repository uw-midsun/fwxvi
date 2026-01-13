#pragma once

/************************************************************************************************
 * @file   rear_controller_hw_defs.h
 *
 * @brief  Rear controller board hardware definitions
 *
 * @date   2025-05-28
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup Rear_Controller
 * @brief    Rear Controller Board Firmware
 * @{
 */

/************************************************************************************************
 * Rear Controller CAN definitions
 ************************************************************************************************/

/** @brief  Rear Controller CAN RX Port */
#define REAR_CONTROLLER_CAN_RX \
  { .port = GPIO_PORT_B, .pin = 8 }

/** @brief  Rear Controller CAN TX Port */
#define REAR_CONTROLLER_CAN_TX \
  { .port = GPIO_PORT_B, .pin = 9 }

/************************************************************************************************
 * Relay definitions
 ************************************************************************************************/

/** @brief  Rear Controller positive relay enable pin */
#define REAR_CONTROLLER_POS_RELAY_ENABLE_GPIO \
  { .port = GPIO_PORT_A, .pin = 6 }

/** @brief  Rear Controller positive relay sense pin */
#define REAR_CONTROLLER_POS_RELAY_SENSE_GPIO \
  { .port = GPIO_PORT_A, .pin = 5 }

/** @brief  Rear Controller negative relay enable pin */
#define REAR_CONTROLLER_NEG_RELAY_ENABLE_GPIO \
  { .port = GPIO_PORT_A, .pin = 4 }

/** @brief  Rear Controller negative relay sense pin */
#define REAR_CONTROLLER_NEG_RELAY_SENSE_GPIO \
  { .port = GPIO_PORT_A, .pin = 3 }

/** @brief  Rear Controller solar relay enable pin */
#define REAR_CONTROLLER_SOLAR_RELAY_ENABLE_GPIO \
  { .port = GPIO_PORT_A, .pin = 8 }

/** @brief  Rear Controller solar relay sense pin */
#define REAR_CONTROLLER_SOLAR_RELAY_SENSE_GPIO \
  { .port = GPIO_PORT_A, .pin = 9 }

/************************************************************************************************
 * Motor interface definitions
 ************************************************************************************************/

/** @brief  Rear Controller motor relay enable pin */
#define REAR_CONTROLLER_MOTOR_RELAY_ENABLE_GPIO \
  { .port = GPIO_PORT_C, .pin = 4 }

/** @brief  Rear Controller motor relay sense pin */
#define REAR_CONTROLLER_MOTOR_RELAY_SENSE_GPIO \
  { .port = GPIO_PORT_A, .pin = 7 }

/** @brief  Rear Controller motor low-voltage enable pin */
#define REAR_CONTROLLER_MOTOR_LV_ENABLE_GPIO \
  { .port = GPIO_PORT_C, .pin = 12 }

/************************************************************************************************
 * Fan definitions
 ************************************************************************************************/

/** @brief  Rear Controller fan pwm */
#define REAR_CONTROLLER_MOTOR_FAN_PWM \
  { .port = GPIO_PORT_B, .pin = 2 }

/** @brief  Rear Controller fan sense 2 */
#define REAR_CONTROLLER_MOTOR_FAN_SENSE_1 \
  { .port = GPIO_PORT_B, .pin = 0 }

/** @brief  Rear Controller fan sense 1 */
#define REAR_CONTROLLER_MOTOR_FAN_SENSE_2 \
  { .port = GPIO_PORT_B, .pin = 1 }

/************************************************************************************************
 * Current sense definitions
 ************************************************************************************************/

/** @brief  Current sense I2C port */
#define REAR_CONTROLLER_CURRENT_SENSE_I2C_PORT I2C_PORT_1

/** @brief  Current sense I2C serial data pin */
#define REAR_CONTROLLER_CURRENT_SENSE_I2C_SDA_GPIO \
  { .port = GPIO_PORT_B, .pin = 11 }

/** @brief  Current sense I2C clock pin */
#define REAR_CONTROLLER_CURRENT_SENSE_I2C_SCL_GPIO \
  { .port = GPIO_PORT_B, .pin = 10 }

/** @brief  Current sense slave address */
#define REAR_CONTROLLER_CURRENT_SENSE_ACS37800_I2C_ADDR (96)

/************************************************************************************************
 * AFEs (LTC6811) SPI definitions
 ************************************************************************************************/

/** @brief  AFE SPI port */
#define REAR_CONTROLLER_ADBMS_AFE_SPI_PORT SPI_PORT_2

/** @brief  AFE SPI Baudrate */
#define REAR_CONTROLLER_ADBMS_AFE_SPI_BAUDRATE SPI_BAUDRATE_312_5KHZ

/** @brief  AFE SPI Serial data out */
#define REAR_CONTROLLER_ADBMS_AFE_SPI_SDO_GPIO \
  { .port = GPIO_PORT_B, .pin = 15 }

/** @brief  AFE SPI Serial data in */
#define REAR_CONTROLLER_ADBMS_AFE_SPI_SDI_GPIO \
  { .port = GPIO_PORT_B, .pin = 14 }

/** @brief  AFE SPI Serial clock line */
#define REAR_CONTROLLER_ADBMS_AFE_SPI_SCK_GPIO \
  { .port = GPIO_PORT_B, .pin = 13 }

/** @brief  AFE SPI Chip select line */
#define REAR_CONTROLLER_ADBMS_AFE_SPI_CS_GPIO \
  { .port = GPIO_PORT_B, .pin = 12 }

/************************************************************************************************
 * Power Sense pins
 ************************************************************************************************/

/** @brief  Rear Controller PCS input valid pin 1 */
#define REAR_CONTROLLER_PCS_VALID1 \
  { .port = GPIO_PORT_C, .pin = 6 }

/** @brief  Rear Controller PCS input valid pin 2 */
#define REAR_CONTROLLER_PCS_VALID2 \
  { .port = GPIO_PORT_C, .pin = 7 }

/** @brief  Rear Controller AUX input valid pin 1 */
#define REAR_CONTROLLER_AUX_VALID1 \
  { .port = GPIO_PORT_C, .pin = 8 }

/** @brief  Rear Controller AUX input valid pin 2 */
#define REAR_CONTROLLER_AUX_VALID2 \
  { .port = GPIO_PORT_C, .pin = 9 }

/** @brief  Rear Controller PCS voltage sense */
#define REAR_CONTROLLER_PCS_VSENSE \
  { .port = GPIO_PORT_A, .pin = 2 }

/** @brief  Rear Controller AUX voltage sense */
#define REAR_CONTROLLER_AUX_VSENSE \
  { .port = GPIO_PORT_A, .pin = 1 }

/** @brief  Rear Controller PCS current sense */
#define REAR_CONTROLLER_PCS_ISENSE \
  { .port = GPIO_PORT_C, .pin = 3 }

/** @brief  Rear Controller AUX current sense */
#define REAR_CONTROLLER_AUX_ISENSE \
  { .port = GPIO_PORT_C, .pin = 2 }

/************************************************************************************************
 * Critical Sense pins
 ************************************************************************************************/

/** @brief  Precharge monitoring pin */
#define REAR_CONTROLLER_PRECHARGE_MONITOR_GPIO \
  { .port = GPIO_PORT_A, .pin = 0 }

/** @brief  Killswitch monitoring pin */
#define REAR_CONTROLLER_KILLSWITCH_MONITOR_GPIO \
  { .port = GPIO_PORT_C, .pin = 5 }

/************************************************************************************************
 * Board LED definition
 ************************************************************************************************/

/** @brief  Rear Controller board LED */
#define REAR_CONTROLLER_BOARD_LED \
  { .port = GPIO_PORT_A, .pin = 10 }

/** @} */
