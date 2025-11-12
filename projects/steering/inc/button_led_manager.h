#pragma once

/************************************************************************************************
 * @file    button_led_manager.h
 *
 * @brief   Button LED manager header file - SK6812 RGB LEDs
 *
 * @date    2025-09-27
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "steering.h"

/**
 * @defgroup steering
 * @brief    steering Firmware
 * @{
 */

#define BUTTON_LED_MANAGER_BITS_PER_LED (24U)
#define BUTTON_LED_MANAGER_DMA_BUF_LEN (512U)

#define BUTTON_LED_MANAGER_COLOR_RED \
  { 255, 0, 0 }
#define BUTTON_LED_MANAGER_COLOR_GREEN \
  { 0, 255, 0 }
#define BUTTON_LED_MANAGER_COLOR_BLUE \
  { 0, 0, 255 }
#define BUTTON_LED_MANAGER_COLOR_YELLOW \
  { 255, 200, 0 }
#define BUTTON_LED_MANAGER_COLOR_ORANGE \
  { 255, 100, 0 }
#define BUTTON_LED_MANAGER_COLOR_PURPLE \
  { 160, 0, 255 }
#define BUTTON_LED_MANAGER_COLOR_CYAN \
  { 0, 255, 255 }
#define BUTTON_LED_MANAGER_COLOR_WHITE \
  { 255, 255, 255 }
#define BUTTON_LED_MANAGER_COLOR_PINK \
  { 255, 80, 150 }
#define BUTTON_LED_MANAGER_COLOR_OFF \
  { 0, 0, 0 }

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} LEDPixels;

typedef struct ButtonLEDManager {
  LEDPixels led_pixels[NUM_STEERING_BUTTONS];          /**< Color data per LED */
  uint16_t dma_buffer[BUTTON_LED_MANAGER_DMA_BUF_LEN]; /**< DMA buffer holds CCR compare values (ticks) */
  bool needs_update;                                   /**< Flag to track if LEDs need updating */
  bool is_transmitting;                                /**< Flag to prevent concurrent transmissions */
} ButtonLEDManager;

/**
 * @brief   Initialize the button LED manager
 * @param   storage Pointer to the SteeringStorage instance
 * @return  STATUS_CODE_OK if initialized successfully
 *          STATUS_CODE_INVALID_ARGS if an invalid parameter is passed in
 */
StatusCode button_led_manager_init(SteeringStorage *storage);

/**
 * @brief   Update a button's LED color (marks for update but doesn't transmit immediately)
 * @param   button Button selector for modification
 * @param   color_code RGB color code for the button LED
 * @return  STATUS_CODE_OK if set successfully
 *          STATUS_CODE_UNINITIALIZED if the system has not already been initialized
 *          STATUS_CODE_INVALID_ARGS if the button is invalid
 */
StatusCode button_led_manager_set_color(SteeringButtons button, LEDPixels color_code);

/**
 * @brief   Force immediate update of all LEDs (call after setting colors)
 * @return  STATUS_CODE_OK if transmission started successfully
 *          STATUS_CODE_UNINITIALIZED if the system has not been initialized
 *          STATUS_CODE_RESOURCE_EXHAUSTED if DMA is busy
 */
StatusCode button_led_manager_update(void);

/**
 * @brief   Check if the LED manager is currently transmitting data
 * @return  true if DMA transmission is in progress, false otherwise
 */
bool button_led_manager_is_busy(void);

/**
 * @brief   Set all LEDs to off (black)
 * @return  STATUS_CODE_OK if successful
 */
StatusCode button_led_manager_clear_all(void);

/** @} */
