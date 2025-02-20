#pragma once

/************************************************************************************************
 * @file   telemetry.h
 *
 * @brief  Header file for telemetry
 *
 * @date   2025-01-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup telemetry
 * @brief    telemetry Firmware
 * @{
 */

/**
 * @brief   Telemetry configuration data
 */
typedef struct {
  UartPort uart_port;                     /**< UART Port for the telemetry interface (XBee, ELRS, etc.) */
  UartSettings uart_settings;             /**< UART Settings for the telemetry interface */
  uint32_t message_transmit_frequency_hz; /**< Transmission frequency of telemetry data packets */
} TelemetryConfig;

/**
 * @brief   Telemetry storage
 */
typedef struct {
  Datagram datagram_buffer[DATAGRAM_BUFFER_SIZE]; /**< Buffer to store the datagram */
  Queue datagram_queue;                           /**< Queue handle for the datagram buffer */
  TelemetryConfig *config;                        /**< Pointer to the telemetry configuration data */
} TelemetryStorage;

/**
 * @brief   Initialize the telemetry interface
 * @param   storage Pointer to the telemetry storage
 * @param   config Pointer to the telemetry config
 * @return  STATUS_CODE_OK if telemetry initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode telemetry_init(TelemetryStorage *storage, TelemetryConfig *config);

/** @} */
