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
 * File-scope definitions
 ************************************************************************************************/

#define STEERING_GPIO_DEF(PORT, PIN) \
  { .port = GPIO_PORT_##PORT, .pin = PIN }

/************************************************************************************************
 * Steering CAN definitions
 ************************************************************************************************/

/** @brief  Steering CAN RX Port */
#define GPIO_STEERING_CAN_RX STEERING_GPIO_DEF(B, 8)

/** @brief  Steering CAN TX Port */
#define GPIO_STEERING_CAN_TX STEERING_GPIO_DEF(B, 9)

/************************************************************************************************
 * Button definitions
 ************************************************************************************************/

/** @brief  Steering left turn button */
#define GPIO_STEERING_LEFT_TURN_BUTTON STEERING_GPIO_DEF(B, 12)

/** @brief  Steering right turn button */
#define GPIO_STEERING_RIGHT_TURN_BUTTON STEERING_GPIO_DEF(A, 6)

/** @brief  Steering hazards button */
#define GPIO_STEERING_HAZARDS_BUTTON STEERING_GPIO_DEF(C, 5)

/** @brief  Steering neutral button */
#define GPIO_STEERING_NEUTRAL_BUTTON STEERING_GPIO_DEF(B, 2)

/** @brief  Steering drive button */
#define GPIO_STEERING_DRIVE_BUTTON STEERING_GPIO_DEF(B, 1)

/** @brief  Steering reverse button */
#define GPIO_STEERING_REVERSE_BUTTON STEERING_GPIO_DEF(E, 7)

/** @brief  Steering horn button */
#define GPIO_STEERING_HORN_BUTTON STEERING_GPIO_DEF(A, 7)

/** @brief  Steering horn button */
#define GPIO_STEERING_REGEN_BUTTON STEERING_GPIO_DEF(E, 8)

/** @brief  Steering cruise control up button */
#define GPIO_STEERING_CC_UP_BUTTON STEERING_GPIO_DEF(B, 13)

/** @brief  Steering cruise control down button */
#define GPIO_STEERING_CC_DOWN_BUTTON STEERING_GPIO_DEF(B, 14)

/************************************************************************************************
 * LED turn signals
 ************************************************************************************************/

/** @brief  Left turn LED */
#define GPIO_STEERING_LEFT_TURN_LED STEERING_GPIO_DEF(B, 6)

/** @brief  Right turn LED */
#define GPIO_STEERING_RIGHT_TURN_LED STEERING_GPIO_DEF(B, 7)

/************************************************************************************************
 * Display control
 ************************************************************************************************/

/** @brief  Display control (Push-pull to control display state) */
#define GPIO_STEERING_DISPLAY_CTRL STEERING_GPIO_DEF(A, 0)
//! On actual board this is PC6 for some reason

/************************************************************************************************
 * Display LTDC Config
 ************************************************************************************************/

/** @brief  LTDC pixel clock pin */
#define GPIO_STEERING_DISPLAY_LTDC_CLOCK STEERING_GPIO_DEF(A, 4)

/** @brief  LTDC horizontal sync pin */
#define GPIO_STEERING_DISPLAY_LTDC_HSYNC STEERING_GPIO_DEF(C, 2)

/** @brief  LTDC vertical sync pin */
#define GPIO_STEERING_DISPLAY_LTDC_VSYNC STEERING_GPIO_DEF(B, 11)

/** @brief  LTDC data enable pin */
#define GPIO_STEERING_DISPLAY_LTDC_DE STEERING_GPIO_DEF(C, 0)

/** @brief  LTDC red channel pins (bits 2-7) */
#define GPIO_STEERING_DISPLAY_LTDC_RED_PINS \
  {                                         \
    {},                           /* R0 */  \
        {},                       /* R1 */  \
        STEERING_GPIO_DEF(E, 15), /* R2 */  \
        STEERING_GPIO_DEF(D, 8),  /* R3 */  \
        STEERING_GPIO_DEF(D, 9),  /* R4 */  \
        STEERING_GPIO_DEF(D, 10), /* R5 */  \
        STEERING_GPIO_DEF(E, 3),  /* R6 */  \
        STEERING_GPIO_DEF(E, 2)   /* R7 */  \
  }

/** @brief  LTDC green channel pins (bits 2-7) */
#define GPIO_STEERING_DISPLAY_LTDC_GREEN_PINS \
  {                                           \
    {},                           /* G0 */    \
        {},                       /* G1 */    \
        STEERING_GPIO_DEF(E, 9),  /* G2 */    \
        STEERING_GPIO_DEF(E, 10), /* G3 */    \
        STEERING_GPIO_DEF(E, 11), /* G4 */    \
        STEERING_GPIO_DEF(E, 12), /* G5 */    \
        STEERING_GPIO_DEF(E, 6),  /* G6 */    \
        STEERING_GPIO_DEF(E, 5)   /* G7 */    \
  }

/** @brief  LTDC blue channel pins (bits 2-7) */
#define GPIO_STEERING_DISPLAY_LTDC_BLUE_PINS \
  {                                          \
    {},                           /* B0 */   \
        {},                       /* B1 */   \
        STEERING_GPIO_DEF(D, 14), /* B2 */   \
        STEERING_GPIO_DEF(D, 15), /* B3 */   \
        STEERING_GPIO_DEF(D, 0),  /* B4 */   \
        STEERING_GPIO_DEF(D, 1),  /* B5 */   \
        STEERING_GPIO_DEF(B, 0),  /* B6 */   \
        STEERING_GPIO_DEF(E, 4)   /* B7 */   \
  }

/************************************************************************************************
 * RGB buttons
 ************************************************************************************************/

/** @brief  PWM pin for RGB LED control */
#define GPIO_STEERING_RGB_LIGHTS_PWM_PIN STEERING_GPIO_DEF(A, 2)

/************************************************************************************************
 * Buzzer
 ************************************************************************************************/

/** @brief  PWM pin for the buzzer */
#define GPIO_STEERING_BUZZER_PWM_PIN STEERING_GPIO_DEF(D, 12)

/** @} */
