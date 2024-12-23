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

/**
 * @brief   Storage class for the device ID and RX data
 */
typedef struct CanStorage {
  volatile CanQueue rx_queue;
  uint16_t device_id;
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
 * @return  STATUS_CODE_OK if adding the filter succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode can_add_filter_in(CanMessageId msg_id);

/**
 * @brief   Initialize the CAN interface
 * @param   msg Pointer to the message to transmit
 * @return  STATUS_CODE_OK if data is transmitted succesfuully
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode can_transmit(const CanMessage *msg);

/**
 * @brief   Initialize the CAN interface
 * @param   msg Pointer to the message to update on receive
 * @return  STATUS_CODE_OK if data is retrieved succesfuully
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode can_receive(const CanMessage *msg);

/**
 * @brief   Run the CAN TX cache to transmit all messages
 * @param   rx_queue Pointer to the CAN RX queue
 * @param   settings Pointer to the CAN settings
 * @return  STATUS_CODE_OK if the cache is cleared succesfuully
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode run_can_tx_cache();

/**
 * @brief   Run the CAN RX cache to receive all messages
 * @param   rx_queue Pointer to the CAN RX queue
 * @param   settings Pointer to the CAN settings
 * @return  STATUS_CODE_OK if the cache is cleared succesfuully
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode run_can_rx_cache();

/**
 * @brief   Clear the RX cache
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode clear_rx_cache();

/**
 * @brief   Clear the TX cache
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode clear_tx_cache();

/** @} */
