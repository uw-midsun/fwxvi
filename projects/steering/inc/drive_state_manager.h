#pragma once

/************************************************************************************************
 * @file   drive_state_manager.h
 *
 * @brief  Header file for drive state manager
 *
 * @date   2025-07-22
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup DRIVE_STATE_MANAGER
 * @brief    Controls drive state for neutral, drive, and reverse.
 * @{
 */

/**
 * @brief Drive states representing the current output
 */
typedef enum {
  DRIVE_STATE_INVALID = 0, /**< Wrong State */
  DRIVE_STATE_NEUTRAL,     /**< Car Not Actively Moving in Neutral */
  DRIVE_STATE_DRIVE,       /**< Car Drive Forward */
  DRIVE_STATE_REVERSE      /**< Car Drive Reverse*/
} DriveState;

/**
 * @brief Requests made to control the drive state
 */
typedef enum {
  DRIVE_STATE_REQUEST_NONE = 0, /**< No Request */
  DRIVE_STATE_REQUEST_D,        /**< Request to switch to drive */
  DRIVE_STATE_REQUEST_N,        /**< Request to switch to neutral */
  DRIVE_STATE_REQUEST_R         /**< Request to switch to reverse */
} DriveStateRequest;

/**
 * @brief Initialize the drive state manager
 */
void drive_state_manager_init(void);

/**
 * @brief Make a request to change the drive state
 * @param req The requested drive state
 */
void drive_state_manager_request(DriveStateRequest req);

/**
 * @brief Update the drive state based on the current request
 */
void drive_state_manager_update(void);

/**
 * @brief Get the current state of the drive controller
 * @return Current drive state
 */
DriveState drive_state_manager_get_state(void);

/** @} */
