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
#include "queues.h"
#include "tasks.h"
#include "uart.h"

/**
 * @defgroup  Logger
 * @brief     Logging library
 * @{
 */

#define MAX_LOG_SIZE (size_t)200
#define LOG_TIMEOUT_MS 10
#define MS_LOG 0

#ifdef STM32L433xx
#define LOG_UART_PORT UART_PORT_1
#define LOG_GPIO_PORT GPIO_PORT_B
#define LOG_TX_PIN 6
#define LOG_RX_PIN 7
#endif

#ifdef STM32L4P5xx
#define LOG_UART_PORT UART_PORT_1
#define LOG_GPIO_PORT GPIO_PORT_A
#define LOG_TX_PIN 9
#define LOG_RX_PIN 10
#endif

#ifdef STM32L496xx
#define LOG_UART_PORT UART_PORT_3
#define LOG_GPIO_PORT GPIO_PORT_C
#define LOG_TX_PIN 10
#define LOG_RX_PIN 11
#endif

#ifndef LOG_UART_PORT
#define LOG_UART_PORT UART_PORT_1
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

typedef struct {
  char log_msg[MAX_LOG_SIZE];
  size_t msg_size;
} logger_message_data;

extern char g_log_buffer[MAX_LOG_SIZE];
extern UartSettings log_uart_settings;
extern Queue s_logger_queue;

#define LOG_DEBUG(fmt, ...) LOG(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) LOG(LOG_LEVEL_CRITICAL, fmt, ##__VA_ARGS__)

// #define task_init() t

// PARAMETERS AND BLOCK COMMENT STUFF <-- may need to move this to a diff location

StatusCode log_init(void);

#ifdef MS_PLATFORM_X86
#define LOG(level, fmt, ...) printf("[%u] %s:%u: " fmt, (level), __FILE__, __LINE__, ##__VA_ARGS__)
#elif MS_DEBUG_LOG
#define LOG(level, fmt, ...)                                                                                                              \
  do {                                                                                                                                    \
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {                                                                              \
      logger_message_data log_data;                                                                                                       \
      log_data.msg_size = (size_t)snprintf(g_log_buffer, MAX_LOG_SIZE, "\r[%u] %s:%u: " fmt, (level), __FILE__, __LINE__, ##__VA_ARGS__); \
      queue_send(&s_logger_queue, &log_data, 1000);                                                                                       \
    }                                                                                                                                     \
  } while (0);
#else
#define LOG(level, fmt, ...) \
  do {                       \
  } while (0);
#endif

/** @} */
