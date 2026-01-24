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
 * Display Control config
 ************************************************************************************************/

/** @brief  Display control (Push-pull to control display state) */
#define STEERING_DISPLAY_CTRL \
  { .port = GPIO_PORT_A, .pin = 0 }
//! On actual board this is PC6 for some reason

/************************************************************************************************
 * Display LTDC Config
 ************************************************************************************************/

/** @brief PWM pin for LTDC */
# define STEERING_DISPLAY_BRIGHTNESS \
  { .port = GPIO_PORT_A, .pin = 1 }

/** @brief  LTDC pixel clock pin */
#define STEERING_DISPLAY_LTDC_CLOCK \
  { .port = GPIO_PORT_A, .pin = 4 }

/** @brief  LTDC horizontal sync pin */
#define STEERING_DISPLAY_LTDC_HSYNC \
  { .port = GPIO_PORT_C, .pin = 2 }

/** @brief  LTDC vertical sync pin */
#define STEERING_DISPLAY_LTDC_VSYNC \
  { .port = GPIO_PORT_B, .pin = 11 }

/** @brief  LTDC data enable pin */
#define STEERING_DISPLAY_LTDC_DE \
  { .port = GPIO_PORT_C, .pin = 0 }

/** @brief  LTDC red channel pins (bits 2-7) */
#define STEERING_DISPLAY_LTDC_RED_PINS               \
  {                                                  \
    {},                                     /* R0 */ \
        {},                                 /* R1 */ \
        { .port = GPIO_PORT_E, .pin = 15 }, /* R2 */ \
        { .port = GPIO_PORT_D, .pin = 8 },  /* R3 */ \
        { .port = GPIO_PORT_D, .pin = 9 },  /* R4 */ \
        { .port = GPIO_PORT_D, .pin = 10 }, /* R5 */ \
        { .port = GPIO_PORT_E, .pin = 3 },  /* R6 */ \
    {                                                \
      .port = GPIO_PORT_E, .pin = 2                  \
    } /* R7 */                                       \
  }

/** @brief  LTDC green channel pins (bits 2-7) */
#define STEERING_DISPLAY_LTDC_GREEN_PINS             \
  {                                                  \
    {},                                     /* G0 */ \
        {},                                 /* G1 */ \
        { .port = GPIO_PORT_E, .pin = 9 },  /* G2 */ \
        { .port = GPIO_PORT_E, .pin = 10 }, /* G3 */ \
        { .port = GPIO_PORT_E, .pin = 11 }, /* G4 */ \
        { .port = GPIO_PORT_E, .pin = 12 }, /* G5 */ \
        { .port = GPIO_PORT_E, .pin = 6 },  /* G6 */ \
    {                                                \
      .port = GPIO_PORT_E, .pin = 5                  \
    } /* G7 */                                       \
  }

/** @brief  LTDC blue channel pins (bits 2-7) */
#define STEERING_DISPLAY_LTDC_BLUE_PINS              \
  {                                                  \
    {},                                     /* B0 */ \
        {},                                 /* B1 */ \
        { .port = GPIO_PORT_D, .pin = 14 }, /* B2 */ \
        { .port = GPIO_PORT_D, .pin = 15 }, /* B3 */ \
        { .port = GPIO_PORT_D, .pin = 0 },  /* B4 */ \
        { .port = GPIO_PORT_D, .pin = 1 },  /* B5 */ \
        { .port = GPIO_PORT_B, .pin = 0 },  /* B6 */ \
    {                                                \
      .port = GPIO_PORT_E, .pin = 4                  \
    } /* B7 */                                       \
  }

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
