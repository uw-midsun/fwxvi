#pragma once

/************************************************************************************************
 * @file   can.h
 *
 * @brief  Header file for CAN Application code
 *
 * @date   2024-11-23
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
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
 * @brief   Initialize the CAN interface
 * @param   rx_queue Pointer to the CAN RX queue
 * @param   settings Pointer to the CAN settings
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode can_add_filter_in();

/**
 * @brief   Initialize the CAN interface
 * @param   rx_queue Pointer to the CAN RX queue
 * @param   settings Pointer to the CAN settings
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode can_transmit();

/**
 * @brief   Initialize the CAN interface
 * @param   rx_queue Pointer to the CAN RX queue
 * @param   settings Pointer to the CAN settings
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode can_receive();

/**
 * @brief   Initialize the CAN interface
 * @param   rx_queue Pointer to the CAN RX queue
 * @param   settings Pointer to the CAN settings
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode cache_can_tx();

/**
 * @brief   Initialize the CAN interface
 * @param   rx_queue Pointer to the CAN RX queue
 * @param   settings Pointer to the CAN settings
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode cache_can_rx();

/**
 * @brief   Initialize the CAN interface
 * @param   rx_queue Pointer to the CAN RX queue
 * @param   settings Pointer to the CAN settings
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode clear_rx_cache();

/**
 * @brief   Initialize the CAN interface
 * @param   rx_queue Pointer to the CAN RX queue
 * @param   settings Pointer to the CAN settings
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode clear_tx_cache();

/** @} */
