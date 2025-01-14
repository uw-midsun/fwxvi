#pragma once

/************************************************************************************************
 * @file   mcp2515_defs.h
 *
 * @brief  Header file for mcdp2515.defs
 *
 * @date   2025-01-13
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
/* Inter-component Headers */
#include "can.h"
#include "mcp2515_defs.h"
#include "mcp2515_hw.h"
/* Intra-component Headers */

/**
 * @defgroup motor_controller
 * @brief    motor_controller Firmware
 * @{
 */

// SPI commands: Table 12-1


extern void (*mcp2515_rx_all)();

extern void (*mcp2515_tx_all)();

// Initializes the specified CAN configuration.
StatusCode mcp2515_init(Mcp2515Storage *storage, const Mcp2515Settings *settings);

// Adds a hardware filter in for the specified message ID.
// StatusCode mcp2515_add_filter_in(CanMessageId msg_id);

// Adds a hardware filter out for the specified message ID.
// StatusCode mcp2515_add_filter_out(CanMessageId msg_id);

StatusCode mcp2515_set_filter(CanMessageId *filters, bool loopback);

// Attempts to transmit the CAN message as soon as possible.
StatusCode mcp2515_transmit(const CanMessage *msg);

// Attempts to receive the CAN message as soon as possible.
StatusCode mcp2515_receive(const CanMessage *msg);

// Run the can rx cycle
StatusCode run_mcp2515_rx_cycle();

// Run the can tx cycle
StatusCode run_mcp2515_tx_cycle();
/** @} */
