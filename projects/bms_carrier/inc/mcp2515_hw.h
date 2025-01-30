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
#include "can_hw.h"
#include "can_msg.h"
#include "can_queue.h"
#include "gpio.h"
#include "spi.h"
/* Intra-component Headers */

/**
 * @defgroup motor_controller
 * @brief    motor_controller Firmware
 * @{
 */

// SPI commands: Table 12-1
typedef struct Mcp2515Errors {
  uint8_t eflg;
  uint8_t tec;
  uint8_t rec;
} Mcp2515Errors;

typedef struct Mcp2515Settings {
  SpiPort spi_port;
  SpiSettings spi_settings;

  GpioAddress interrupt_pin;
  GpioAddress RX0BF;
  GpioAddress RX1BF;

  // same can settings except MCP2515 does not support 1000kbps bitrate
  CanSettings can_settings;
} Mcp2515Settings;

typedef struct Mcp2515Storage {
  SpiPort spi_port;
  volatile CanQueue rx_queue;
  Mcp2515Errors errors;
  bool loopback;
} Mcp2515Storage;

// Initializes CAN using the specified settings.
StatusCode mcp2515_hw_init(Mcp2515Storage *rx_queue, const Mcp2515Settings *settings);

// StatusCode mcp2515_hw_add_filter_in(uint32_t mask, uint32_t filter, bool extended);

StatusCode mcp2515_hw_set_filter(CanMessageId *filters, bool loopback);

CanHwBusStatus mcp2515_hw_bus_status(void);

StatusCode mcp2515_hw_transmit(uint32_t id, bool extended, uint8_t *data, size_t len);
/** @} */
