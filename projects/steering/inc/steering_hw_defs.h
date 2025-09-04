#pragma once

/************************************************************************************************
 * @file   steering_hw_defs.h
 *
 * @brief  Header file for Steering hardware definitions
 *
 * @date   2025-01-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup steering
 * @brief    steering Firmware
 * @{
 */

/************************************************************************************************
 * Steering CAN definitions
 ************************************************************************************************/

/** @brief  Steering CAN RX Port */
#define STEERING_CAN_RX { .port = GPIO_PORT_B, .pin = 8 }

/** @brief  Steering CAN TX Port */
#define STEERING_CAN_TX { .port = GPIO_PORT_B, .pin = 9 }

/************************************************************************************************
 * Button definitions
 ************************************************************************************************/

/** @brief  Steering left turn button */
#define STEERING_LEFT_TURN_BUTTON { .port = GPIO_PORT_B, .pin = 3 }

/** @brief  Steering right turn button */
#define STEERING_RIGHT_TURN_BUTTON { .port = GPIO_PORT_B, .pin = 4 }

/** @brief  Steering hazards button */
#define STEERING_HAZARDS_BUTTON { .port = GPIO_PORT_B, .pin = 5 }

/** @brief  Steering neutral button */
#define STEERING_NEUTRAL_BUTTON { .port = GPIO_PORT_A, .pin = 5 }

/** @brief  Steering drive button */
#define STEERING_DRIVE_BUTTON { .port = GPIO_PORT_A, .pin = 6 }

/** @brief  Steering reverse button */
#define STEERING_REVERSE_BUTTON { .port = GPIO_PORT_A, .pin = 7 }

/** @brief  Steering horn button */
#define STEERING_HORN_BUTTON { .port = GPIO_PORT_B, .pin = 6 }

/************************************************************************************************
 * Display control
 ************************************************************************************************/

/** @brief  Display control (Push-pull to control display state) */
#define STEERING_DISPLAY_CTRL { .port = GPIO_PORT_A, .pin = 0 }

/** @brief  Display current control (PWM to control % of current) */
#define STEERING_DISPLAY_CURRENT_CTRL { .port = GPIO_PORT_A, .pin = 1 }

/************************************************************************************************
 * RGB buttons
 ************************************************************************************************/

/** @brief  PWM pin for RGB LED control */
#define STEERING_RGB_LIGHTS_PWM_PIN { .port = GPIO_PORT_A, .pin = 2 }

/** @} */
