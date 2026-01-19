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
#define STEERING_CAN_RX \
  { .port = GPIO_PORT_B, .pin = 8 }

/** @brief  Steering CAN TX Port */
#define STEERING_CAN_TX \
  { .port = GPIO_PORT_B, .pin = 9 }

/************************************************************************************************
 * Button definitions
 ************************************************************************************************/

/** @brief  Steering left turn button */
#define STEERING_LEFT_TURN_BUTTON \
  { .port = GPIO_PORT_B, .pin = 12 }

/** @brief  Steering right turn button */
#define STEERING_RIGHT_TURN_BUTTON \
  { .port = GPIO_PORT_A, .pin = 6 }

/** @brief  Steering hazards button */
#define STEERING_HAZARDS_BUTTON \
  { .port = GPIO_PORT_C, .pin = 5 }

/** @brief  Steering neutral button */
#define STEERING_NEUTRAL_BUTTON \
  { .port = GPIO_PORT_B, .pin = 2 }

/** @brief  Steering drive button */
#define STEERING_DRIVE_BUTTON \
  { .port = GPIO_PORT_B, .pin = 1 }

/** @brief  Steering reverse button */
#define STEERING_REVERSE_BUTTON \
  { .port = GPIO_PORT_E, .pin = 7 }

/** @brief  Steering horn button */
#define STEERING_HORN_BUTTON \
  { .port = GPIO_PORT_A, .pin = 7 }

/** @brief  Steering horn button */
#define STEERING_REGEN_BUTTON \
  { .port = GPIO_PORT_E, .pin = 8 }

/** @brief  Steering cruise control up button */
#define STEERING_CC_UP_BUTTON \
  { .port = GPIO_PORT_B, .pin = 13 }

/** @brief  Steering cruise control down button */
#define STEERING_CC_DOWN_BUTTON \
  { .port = GPIO_PORT_B, .pin = 14 }

/************************************************************************************************
 * LED turn signals
 ************************************************************************************************/

/** @brief  Left turn LED */
#define STEERING_LEFT_TURN_LED \
  { .port = GPIO_PORT_B, .pin = 6 }

/** @brief  Right turn LED */
#define STEERING_RIGHT_TURN_LED \
  { .port = GPIO_PORT_B, .pin = 7 }

/************************************************************************************************
 * Display control
 ************************************************************************************************/

/** @brief  Display control (Push-pull to control display state) */
#define STEERING_DISPLAY_CTRL \
  { .port = GPIO_PORT_A, .pin = 0 }

/** @brief  Display current control (PWM to control % of current) */
#define STEERING_DISPLAY_CURRENT_CTRL \
  { .port = GPIO_PORT_A, .pin = 1 }

/************************************************************************************************
 * RGB buttons
 ************************************************************************************************/

/** @brief  PWM pin for RGB LED control */
#define STEERING_RGB_LIGHTS_PWM_PIN \
  { .port = GPIO_PORT_A, .pin = 2 }

/************************************************************************************************
 * Buzzer
 ************************************************************************************************/

/** @brief  PWM pin for the buzzer */
#define STEERING_BUZZER_PWM_PIN \
  { .port = GPIO_PORT_D, .pin = 12 }

/** @} */
