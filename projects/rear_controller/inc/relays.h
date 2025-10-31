#pragma once

/************************************************************************************************
 * @file    relays.h
 *
 * @brief   Relays control header file
 *
 * @date    2025-10-12
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "gpio.h"
#include "status.h"

/* Intra-component Headers */
#include "rear_controller.h"
#include "rear_controller_hw_defs.h"

/**
 * @defgroup Rear_Controller
 * @brief    Rear Controller Board Firmware
 * @{
 */

#define NUM_REAR_RELAYS 4U
#define REAR_CLOSE_RELAYS_DELAY_MS 250U

#define REAR_CLOSE_RELAYS_DELAY_MS 250U

/**
 * @brief   Initialize the relay control module
 * @param   storage Pointer to the rear controller storage
 * @return  STATUS_CODE_OK
 */
StatusCode relays_init(RearControllerStorage *storage);

/**
 * @brief   Reset relays by disengaging all relays
 * @return  STATUS_CODE_OK if relays opened successfully
 */
StatusCode relays_reset();

/**
 * @brief   Close the high-side (POS) relay
 * @return  STATUS_CODE_OK if relay closed successfully
 */
StatusCode relays_close_pos(void);

/**
 * @brief   Open the high-side (POS) relay
 * @return  STATUS_CODE_OK if relay opened successfully
 */
StatusCode relays_open_pos(void);

/**
 * @brief   Close the low-side (NEG) relay
 * @return  STATUS_CODE_OK if relay closed successfully
 */
StatusCode relays_close_neg(void);

/**
 * @brief   Open the low-side (NEG) relay
 * @return  STATUS_CODE_OK if relay opened successfully
 */
StatusCode relays_open_neg(void);

/**
 * @brief   Close the motor HV relay
 * @return  STATUS_CODE_OK if relay closed successfully
 */
StatusCode relays_close_motor(void);

/**
 * @brief   Open the motor HV relay
 * @return  STATUS_CODE_OK if relay opened successfully
 */
StatusCode relays_open_motor(void);

/**
 * @brief   Close the solar array relay
 * @return  STATUS_CODE_OK if relay closed successfully
 */
StatusCode relays_close_solar(void);

/**
 * @brief   Open the solar array relay
 * @return  STATUS_CODE_OK if relay opened successfully
 */
StatusCode relays_open_solar(void);

/** @} */
