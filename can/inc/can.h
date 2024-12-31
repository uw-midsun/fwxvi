#pragma once

/************************************************************************************************
 * @file   can.h
 *
 * @brief  Header file for CAN Application code
 *
 * @date   2024-11-23
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "can_hw.h"
#include "can_queue.h"

/**
 * @defgroup CAN
 * @brief    CAN library
 * @{
 */

/** @brief Maximum time permitted for a CAN transaction */
#define CAN_TIMEOUT_MS      5U

/**
 * @brief   Storage class for the device ID and RX data
 */
typedef struct CanStorage {
  volatile CanQueue rx_queue;   /**< Global RX queue to store CAN messages */
  uint16_t device_id;           /**< Device ID of the running device */
} CanStorage;

/**
 * @brief   Initialize the CAN interface
 * @param   storage Pointer to the CAN storage
 * @param   settings Pointer to the CAN settings
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if initialization fails
 */
StatusCode can_init(CanStorage *storage, const CanSettings *settings);

/**
 * @brief   Sets a filter on the CAN interface
 * @param   msg_id Message ID of the message to filter
 * @return  STATUS_CODE_OK if the filter is added successfully
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode can_add_filter_in(CanMessageId msg_id);

/**
 * @brief   Transmits CAN data on the bus
 * @param   msg Pointer to the message to transmit
 * @return  STATUS_CODE_OK if data is transmitted successfully
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_UNINITIALIZED if the CAN bus is not initialized
 */
StatusCode can_transmit(const CanMessage *msg);

/**
 * @brief   Receives CAN data from the bus
 * @param   msg Pointer to the message to update on receive
 * @return  STATUS_CODE_OK if data is retrieved successfully
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_UNINITIALIZED if the CAN bus is not initialized
 */
StatusCode can_receive(const CanMessage *msg);

/**
 * @brief   Transmit all CAN data
 * @return  STATUS_CODE_OK if the cache is cleared successfully
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode run_can_tx_all();

/**
 * @brief   Transmit all fast-cycle CAN data
 * @return  STATUS_CODE_OK if the cache is cleared successfully
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode run_can_tx_fast();

/**
 * @brief   Transmit all medium-cycle CAN data
 * @return  STATUS_CODE_OK if the cache is cleared successfully
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode run_can_tx_medium();

/**
 * @brief   Transmit all slow-cycle CAN data
 * @return  STATUS_CODE_OK if the cache is cleared successfully
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode run_can_tx_slow();

/**
 * @brief   Receive all CAN data
 * @return  STATUS_CODE_OK if the cache is cleared successfully
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode run_can_rx_all();

/**
 * @brief   Clear the RX data struct
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode clear_rx_struct();

/**
 * @brief   Clear the TX data struct
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode clear_tx_struct();

/** @} */
