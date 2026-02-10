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
 * Telemetry UART definitions
 ************************************************************************************************/

/** @brief  Telemetry UART RX Port */
#define GPIO_TELEMETRY_UART_RX TELEMETRY_GPIO_DEF(A, 2)

/** @brief  Telemetry UART TX Port */
#define GPIO_TELEMETRY_UART_TX TELEMETRY_GPIO_DEF(A, 3)

/************************************************************************************************
 * Board LED definition
 ************************************************************************************************/

/** @brief  Telemetry board LED */
#define GPIO_TELEMETRY_BOARD_LED TELEMETRY_GPIO_DEF(A, 6)

/** @} */
