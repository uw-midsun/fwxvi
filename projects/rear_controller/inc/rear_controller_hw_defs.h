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
 * File-scope definitions
 ************************************************************************************************/

#define REAR_CONTROLLER_GPIO_DEF(PORT, PIN) { .port = GPIO_PORT_##PORT, .pin = PIN }

/************************************************************************************************
 * Rear Controller CAN definitions
 ************************************************************************************************/

/** @brief  Rear Controller CAN RX Port */
#define GPIO_REAR_CONTROLLER_CAN_RX REAR_CONTROLLER_GPIO_DEF(B, 8)

/** @brief  Rear Controller CAN TX Port */
#define GPIO_REAR_CONTROLLER_CAN_TX REAR_CONTROLLER_GPIO_DEF(B, 9)

/************************************************************************************************
 * Relay definitions
 ************************************************************************************************/

/** @brief  Rear Controller positive relay enable pin */
#define GPIO_REAR_CONTROLLER_POS_RELAY_ENABLE REAR_CONTROLLER_GPIO_DEF(A, 6)

/** @brief  Rear Controller positive relay sense pin */
#define GPIO_REAR_CONTROLLER_POS_RELAY_SENSE REAR_CONTROLLER_GPIO_DEF(A, 5)

/** @brief  Rear Controller negative relay enable pin */
#define GPIO_REAR_CONTROLLER_NEG_RELAY_ENABLE REAR_CONTROLLER_GPIO_DEF(A, 4)

/** @brief  Rear Controller negative relay sense pin */
#define GPIO_REAR_CONTROLLER_NEG_RELAY_SENSE REAR_CONTROLLER_GPIO_DEF(A, 3)

/** @brief  Rear Controller solar relay enable pin */
#define GPIO_REAR_CONTROLLER_SOLAR_RELAY_ENABLE REAR_CONTROLLER_GPIO_DEF(A, 8)

/** @brief  Rear Controller solar relay sense pin */
#define GPIO_REAR_CONTROLLER_SOLAR_RELAY_SENSE REAR_CONTROLLER_GPIO_DEF(A, 9)

/************************************************************************************************
 * Motor interface definitions
 ************************************************************************************************/

/** @brief  Rear Controller motor relay enable pin */
#define GPIO_REAR_CONTROLLER_MOTOR_RELAY_ENABLE REAR_CONTROLLER_GPIO_DEF(C, 4)

/** @brief  Rear Controller motor relay sense pin */
#define GPIO_REAR_CONTROLLER_MOTOR_RELAY_SENSE REAR_CONTROLLER_GPIO_DEF(A, 7)

/** @brief  Rear Controller motor low-voltage enable pin */
#define GPIO_REAR_CONTROLLER_MOTOR_LV_ENABLE REAR_CONTROLLER_GPIO_DEF(C, 12)

/************************************************************************************************
 * Fan definitions
 ************************************************************************************************/

/** @brief  Rear Controller fan pwm */
#define GPIO_REAR_CONTROLLER_MOTOR_FAN_PWM REAR_CONTROLLER_GPIO_DEF(B, 2)

/** @brief  Rear Controller fan sense 2 */
#define GPIO_REAR_CONTROLLER_MOTOR_FAN_SENSE_1 REAR_CONTROLLER_GPIO_DEF(B, 0)

/** @brief  Rear Controller fan sense 1 */
#define GPIO_REAR_CONTROLLER_MOTOR_FAN_SENSE_2 REAR_CONTROLLER_GPIO_DEF(B, 1)

/************************************************************************************************
 * Current sense definitions
 ************************************************************************************************/

/** @brief  Current sense I2C port */
#define REAR_CONTROLLER_CURRENT_SENSE_I2C_PORT I2C_PORT_1

/** @brief  Current sense I2C serial data pin */
#define GPIO_REAR_CONTROLLER_CURRENT_SENSE_I2C_SDA_GPIO REAR_CONTROLLER_GPIO_DEF(B, 11)

/** @brief  Current sense I2C clock pin */
#define GPIO_REAR_CONTROLLER_CURRENT_SENSE_I2C_SCL_GPIO REAR_CONTROLLER_GPIO_DEF(B, 10)

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
#define GPIO_REAR_CONTROLLER_ADBMS_AFE_SPI_SDO_GPIO REAR_CONTROLLER_GPIO_DEF(B, 15)

/** @brief  AFE SPI Serial data in */
#define GPIO_REAR_CONTROLLER_ADBMS_AFE_SPI_SDI_GPIO REAR_CONTROLLER_GPIO_DEF(B, 14)

/** @brief  AFE SPI Serial clock line */
#define GPIO_REAR_CONTROLLER_ADBMS_AFE_SPI_SCK_GPIO REAR_CONTROLLER_GPIO_DEF(B, 13)

/** @brief  AFE SPI Chip select line */
#define GPIO_REAR_CONTROLLER_ADBMS_AFE_SPI_CS_GPIO REAR_CONTROLLER_GPIO_DEF(B, 12)

/************************************************************************************************
 * Power Sense pins
 ************************************************************************************************/

/** @brief  Rear Controller PCS input valid pin 1 */
#define GPIO_REAR_CONTROLLER_PCS_VALID1 REAR_CONTROLLER_GPIO_DEF(C, 6)

/** @brief  Rear Controller PCS input valid pin 2 */
#define GPIO_REAR_CONTROLLER_PCS_VALID2 REAR_CONTROLLER_GPIO_DEF(C, 7)

/** @brief  Rear Controller AUX input valid pin 1 */
#define GPIO_REAR_CONTROLLER_AUX_VALID1 REAR_CONTROLLER_GPIO_DEF(C, 8)

/** @brief  Rear Controller AUX input valid pin 2 */
#define GPIO_REAR_CONTROLLER_AUX_VALID2 REAR_CONTROLLER_GPIO_DEF(C, 9)

/** @brief  Rear Controller PCS voltage sense */
#define GPIO_REAR_CONTROLLER_PCS_VSENSE REAR_CONTROLLER_GPIO_DEF(A, 2)

/** @brief  Rear Controller AUX voltage sense */
#define GPIO_REAR_CONTROLLER_AUX_VSENSE REAR_CONTROLLER_GPIO_DEF(A, 1)

/** @brief  Rear Controller PCS current sense */
#define GPIO_REAR_CONTROLLER_PCS_ISENSE REAR_CONTROLLER_GPIO_DEF(C, 3)

/** @brief  Rear Controller AUX current sense */
#define GPIO_REAR_CONTROLLER_AUX_ISENSE REAR_CONTROLLER_GPIO_DEF(C, 2)

/************************************************************************************************
 * Critical Sense pins
 ************************************************************************************************/

/** @brief  Precharge monitoring pin */
#define GPIO_REAR_CONTROLLER_PRECHARGE_MONITOR_GPIO REAR_CONTROLLER_GPIO_DEF(A, 0)

/** @brief  Killswitch monitoring pin */
#define GPIO_REAR_CONTROLLER_KILLSWITCH_MONITOR REAR_CONTROLLER_GPIO_DEF(C, 5)

/************************************************************************************************
 * Board LED definition
 ************************************************************************************************/

/** @brief  Rear Controller board LED */
#define GPIO_REAR_CONTROLLER_BOARD_LED REAR_CONTROLLER_GPIO_DEF(A, 10)

/** @} */
