#pragma once

/************************************************************************************************
 * @file   bms_hw_defs.h
 *
 * @brief  Header file for BMS hardware definitions
 *
 * @date   2025-01-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "bms_carrier.h"
#include "can.h"
#include "delay.h"
// #include "exported_enums.h"
#include "gpio.h"
#include "gpio_interrupts.h"
#include "interrupts.h"
#include "log.h"
#include "master_tasks.h"
#include "status.h"
#include "task.h"
#include "timers.h"

/* Intra-component Headers */

/**
 * @defgroup bms_carrier
 * @brief    bms_carrier Firmware
 * @{
 */

#define BMS_CLOSE_RELAYS_DELAY_MS 200
#define KILLSWITCH_IT 3

// Enumerated set of relays to be closed
typedef enum RelayType {
  NO_RELAYS = 0,
  POS_RELAY,
  NEG_RELAY,
  SOLAR_RELAY,
  RELAY_CHECK,
} RelayType;

// Closes relays in sequence
StatusCode init_bms_relays(GpioAddress *killswitch);

// Independent solar control
void bms_open_solar();
void bms_close_solar();

// Turns off GPIOs to open relays
void bms_relay_fault(void);
/** @} */
