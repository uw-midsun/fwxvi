#pragma once

/************************************************************************************************
 * @file   telemetry_hw_defs.h
 *
 * @brief  Header file for Telemetry HW definition
 *
 * @date   2026-02-09
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup Telemetry
 * @brief    Telemetry Board Firmware
 * @{
 */
/************************************************************************************************
 * File-scope definitions
 ************************************************************************************************/

#define TELEMETRY_GPIO_DEF(PORT, PIN) { .port = GPIO_PORT_##PORT, .pin = PIN }

/************************************************************************************************
 * Telemetry CAN definitions
 ************************************************************************************************/

/** @brief  Telemetry CAN RX Port */
#define GPIO_TELEMETRY_CAN_RX TELEMETRY_GPIO_DEF(B, 9)

/** @brief  Telemetry CAN TX Port */
#define GPIO_TELEMETRY_CAN_TX TELEMETRY_GPIO_DEF(B, 8)

/************************************************************************************************
 * Telemetry UART definitions - for XBee transciever
 ************************************************************************************************/

/** @brief  Telemetry UART RX Port */
#define GPIO_TELEMETRY_UART_RX TELEMETRY_GPIO_DEF(A, 2)

/** @brief  Telemetry UART TX Port */
#define GPIO_TELEMETRY_UART_TX TELEMETRY_GPIO_DEF(A, 3)

/** @brief  Telemetry XBee XRST Pin */
#define GPIO_TELEMETRY_XBEE_XRST TELEMETRY_GPIO_DEF(A, 5)

/** @brief  Telemetry XBee SLEEP_RQ Pin */
#define GPIO_TELEMETRY_XBEE_SLEEP_RQ TELEMETRY_GPIO_DEF(A, 11)

/************************************************************************************************
 * Telemetry SPI definitions - for SD card & IMU
 ************************************************************************************************/

/** @brief  Telemetry SPI MISO Port */
#define GPIO_TELEMETRY_SPI_MISO TELEMETRY_GPIO_DEF(B, 14)

/** @brief  Telemetry SPI MOSI Port */
#define GPIO_TELEMETRY_SPI_MOSI TELEMETRY_GPIO_DEF(B, 15)

/** @brief  Telemetry SPI SCK Port */
#define GPIO_TELEMETRY_SPI_SCK TELEMETRY_GPIO_DEF(B, 13)

/** @brief  Telemetry SPI NSS Port */
#define GPIO_TELEMETRY_SPI_NSS TELEMETRY_GPIO_DEF(B, 12)

/************************************************************************************************
 * Telemetry IMU-specific Definitions
 ************************************************************************************************/

/** @brief  Telemetry IMU INTL1 Pin */
#define GPIO_TELEMETRY_IMU_INTL1 TELEMETRY_GPIO_DEF(B, 0)

/** @brief  Telemetry IMU INTL2 Pin */
#define GPIO_TELEMETRY_IMU_INTL2 TELEMETRY_GPIO_DEF(B, 1)

/************************************************************************************************
 * Telemetry SD-card-specific Definitions
 ************************************************************************************************/

/** @brief  Telemetry SD card CD_B Pin (chip-detect) */
#define GPIO_TELEMETRY_SD_CARD_CD_B TELEMETRY_GPIO_DEF(B, 10)

/************************************************************************************************
 * Board LED definition
 ************************************************************************************************/

/** @brief  Telemetry board LED */
#define GPIO_TELEMETRY_BOARD_LED TELEMETRY_GPIO_DEF(A, 6)

/** @} */
