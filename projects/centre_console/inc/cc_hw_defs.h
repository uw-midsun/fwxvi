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

#define ADC_HALL_SENSOR_PIN { .port = GPIO_PORT_A, .pin = 0 }

#define BUTTONS_SDA { .port = GPIO_PORT_A, .pin = 10 }

#define BUTTONS_SCL { .port = GPIO_PORT_A, .pin = 9 }

#define LED_BPS { .port = GPIO_PORT_A, .pin = 8 }

#define ADC_TURN_SIGNAL_PIN { .port = GPIO_PORT_A, .pin = 6 }

#define ADC_CRUISE_CONTROL_PIN { .port = GPIO_PORT_A, .pin = 7 }

#define BUZZER_PIN { .port = GPIO_PORT_B, .pin = 15 }

#define LIM_S_BRAKE_PIN { .port = GPIO_PORT_B, .pin = 13 }

#define CRUISE_CONTROL_BUTTON_PIN { .port = GPIO_PORT_B, .pin = 12 }

/** @} */
