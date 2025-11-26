#pragma once

/************************************************************************************************
 * @file   log.h
 *
 * @brief  Header file for the logging library used to debug all modules
 *
 * @date   2024-11-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdio.h>
#include <stdlib.h>

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "task.h"

/* Intra-component Headers */
#include "gpio.h"
#include "ms_semaphore.h"
#include "tasks.h"
#include "uart.h"

/**
 * @defgroup  Logger
 * @brief     Logging library
 * @{
 */

#define MAX_LOG_SIZE (size_t)200
#define LOG_TIMEOUT_MS 10

#define LOG_UART_PORT UART_PORT_1

#ifdef STM32L433xx
#define LOG_GPIO_PORT GPIO_PORT_B
#define LOG_TX_PIN 6
#define LOG_RX_PIN 7
#endif

#ifdef STM32L4P5xx
#define LOG_GPIO_PORT GPIO_PORT_A
#define LOG_TX_PIN 9
#define LOG_RX_PIN 10
#endif

#ifdef STM32L496xx
#define LOG_GPIO_PORT GPIO_PORT_C
#define LOG_TX_PIN 10
#define LOG_RX_PIN 11
#endif

#ifndef LOG_GPIO_PORT
#define LOG_GPIO_PORT GPIO_PORT_B
#endif

#ifndef LOG_TX_PIN
#define LOG_TX_PIN 6
#endif

#ifndef LOG_RX_PIN
#define LOG_RX_PIN 7
#endif

#ifndef IN_ORDER_LOGS
#define IN_ORDER_LOGS 1
#endif

typedef enum {
  LOG_LEVEL_DEBUG = 0, /**< Debug level */
  LOG_LEVEL_WARN,      /**< Warning level */
  LOG_LEVEL_CRITICAL,  /**< Critical level */
  NUM_LOG_LEVELS,      /**< Number of Log levels */
} LogLevel;

extern char g_log_buffer[MAX_LOG_SIZE];
extern Mutex s_log_mutex;
extern UartSettings log_uart_settings;

#define LOG_DEBUG(fmt, ...) LOG(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) LOG(LOG_LEVEL_CRITICAL, fmt, ##__VA_ARGS__)

#define log_init() \
  { uart_init(LOG_UART_PORT, &log_uart_settings); }

#ifdef MS_PLATFORM_X86
#define LOG(level, fmt, ...) printf("[%u] %s:%u: " fmt, (level), __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG(level, fmt, ...)                                                                                                            \
  do {                                                                                                                                  \
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {                                                                            \
      size_t msg_size = (size_t)snprintf(g_log_buffer, MAX_LOG_SIZE, "\r[%u] %s:%u: " fmt, (level), __FILE__, __LINE__, ##__VA_ARGS__); \
      uart_tx(LOG_UART_PORT, (uint8_t *)g_log_buffer, msg_size);                                                                        \
    }                                                                                                                                   \
  } while (0)
#endif

/** @} */
