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
#include <stdbool.h>
/* Inter-component Headers */
#include "gpio.h"
#include "notify.h"
#include "status.h"
/* Intra-component Headers */

/**
 * @defgroup motor_controller
 * @brief    motor_controller Firmware
 * @{
 */

// SPI commands: Table 12-1


// Requires GPIO to be initialized
// Requires GPIO interrupts to be initialized

#define PRECHARGE_EVENT 0

typedef struct PrechargeSettings {
  GpioAddress motor_sw;
  GpioAddress precharge_monitor;
} PrechargeSettings;

StatusCode precharge_init(const PrechargeSettings *settings, Event event, const Task *task);
/** @} */


