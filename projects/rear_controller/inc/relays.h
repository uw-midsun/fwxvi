#pragma once

/************************************************************************************************
 * @file    relays.h
 *
 * @brief   Relays
 *
 * @date    2025-10-12
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "gpio.h"
#include "rear_controller.h"
#include "rear_controller_hw_defs.h"

/* Intra-component Headers */
#include "status.h"

/**
 * @defgroup relays
 * @brief    relays Firmware
 * @{
 */

#define NUM_REAR_RELAYS 4U
#define REAR_CLOSE_RELAYS_DELAY_MS 250U
#define KILLSWICTH_EVENT_IT 3U

#define REAR_CLOSE_RELAYS_DELAY_MS 250U

StatusCode relays_init(RearControllerStorage *storage);
StatusCode relays_fault();
StatusCode relays_close_pos(void);
StatusCode relays_open_pos(void);
StatusCode relays_close_neg(void);
StatusCode relays_open_neg(void);
StatusCode relays_close_motor();
StatusCode relays_open_motor();
StatusCode relays_close_solar();
StatusCode relays_open_solar();
bool relays_is_killswitch_active(void);
StatusCode relays_verify_states(void);

StatusCode rear_controller_deinit(void);
/** @} */
