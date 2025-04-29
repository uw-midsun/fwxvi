#pragma once

/************************************************************************************************
 * @file   can_watchdog.h
 *
 * @brief  Software watchdog library for the CAN bus
 *
 * @date   2024-12-23
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "log.h"

/* Intra-component Headers */
#include "can.h"

/**
 * @defgroup CAN
 * @brief    CAN library
 * @{
 */

/**
 * @brief   Software defined CAN watchdog
 */
typedef struct CanWatchDog {
  uint16_t cycles_over;         /**< Tracks the number of cycles over max_cycles */
  uint16_t max_cycles;          /**< Maximum number of cycles open for data to be received */
  bool missed;                  /**< Flag to track if the message has been missed */
} CanWatchDog;

/**
 * @brief   Checks all CAN watchdogs
 * @details This will throw a LOG_WARN if a message has missed its watchdog
 * @return  STATUS_CODE_OK if all watchdogs are checked succesfully
 */
StatusCode check_all_can_watchdogs();

/**
 * @brief   Checks CAN watchdogs for messages in fast-cycle
 * @details This will throw a LOG_WARN if a message has missed its watchdog
 * @return  STATUS_CODE_OK if all watchdogs are checked succesfully
 */
StatusCode check_fast_can_watchdogs();

/**
 * @brief   Checks CAN watchdogs for messages in medium-cycle
 * @details This will throw a LOG_WARN if a message has missed its watchdog
 * @return  STATUS_CODE_OK if all watchdogs are checked succesfully
 */
StatusCode check_medium_can_watchdogs();

/**
 * @brief   Checks CAN watchdogs for messages in slow-cycle
 * @details This will throw a LOG_WARN if a message has missed its watchdog
 * @return  STATUS_CODE_OK if all watchdogs are checked succesfully
 */
StatusCode check_slow_can_watchdogs();

void clear_all_rx_received();
void clear_fast_rx_received();
void clear_medium_rx_received();
void clear_slow_rx_received();

/** @} */
