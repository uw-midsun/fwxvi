#pragma once

/************************************************************************************************
 * @file   cc_hw_defs.h
 *
 * @brief  Header file for Centre Console hardware definitions
 *
 * @date   2025-01-30
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "gpio.h"
#include "i2c.h"

/* Intra-component Headers */

/**
 * @defgroup centre_console
 * @brief    centre_console Firmware
 * @{
 */

#define ADC_HALL_SENSOR { .port = GPIO_PORT_A, .pin = 0 }

#define BUTTONS_SDA { .port = GPIO_PORT_A, .pin = 10 }

#define BUTTONS_SCL { .port = GPIO_PORT_A, .pin = 9 }

#define LED_BPS { .port = GPIO_PORT_A, .pin = 8 }

#define ADC_TURN_SIGNAL { .port = GPIO_PORT_A, .pin = 6 }

#define ADC_CRUISE_CONTROL { .port = GPIO_PORT_A, .pin = 7 }

#define BUZZER { .port = GPIO_PORT_B, .pin = 15 }

#define BRAKE_LIMIT_SWITCH { .port = GPIO_PORT_B, .pin = 13 }

#define CRUISE_CONTROL_BUTTON { .port = GPIO_PORT_B, .pin = 12 }

/** @} */
