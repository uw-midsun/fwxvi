/************************************************************************************************
 * @file    button_led_manager.h
 *
 * @brief   Button LED manager source file
 *
 * @date    2025-09-27
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#include "gpio.h"

/* Intra-component Headers */
#include "button_led_manager.h"
#include "steering.h"
#include "steering_hw_defs.h"

static SteeringStorage *steering_storage;

static ButtonLEDManager s_button_led_manager = { 0U };

static GpioAddress s_button_led_pwm_ctrl = STEERING_RGB_LIGHTS_PWM_PIN;

StatusCode button_led_manager_init(SteeringStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  steering_storage = storage;
  steering_storage->button_led_manager = &s_button_led_manager;

  /* Clear all LEDs initially */
  memset(&s_button_led_manager.led_pixels, 0, sizeof(s_button_led_manager.led_pixels));
  s_button_led_manager.needs_update = false;
  s_button_led_manager.is_transmitting = false;

  /* Initialize GPIO pin for PWM output */
  StatusCode status = gpio_init_pin_af(&s_button_led_pwm_ctrl, GPIO_ALTFN_PUSH_PULL, GPIO_ALT1_TIM2);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  return STATUS_CODE_OK;
}

StatusCode button_led_manager_set_color(SteeringButtons button, LEDPixels color_code) {
  if (steering_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (button >= NUM_STEERING_BUTTONS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  memcpy(&steering_storage->button_led_manager->led_pixels[button], &color_code, sizeof(LEDPixels));
  steering_storage->button_led_manager->needs_update = true;

  return STATUS_CODE_OK;
}

StatusCode button_led_manager_update(void) {
  if (steering_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (steering_storage->button_led_manager->is_transmitting) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  return STATUS_CODE_OK;
}

bool button_led_manager_is_busy(void) {
  if (steering_storage == NULL) {
    return false;
  }

  return steering_storage->button_led_manager->is_transmitting;
}

StatusCode button_led_manager_clear_all(void) {
  if (steering_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  /* Set all LEDs to black (off) */
  memset(steering_storage->button_led_manager->led_pixels, 0U, sizeof(steering_storage->button_led_manager->led_pixels));
  steering_storage->button_led_manager->needs_update = true;

  return STATUS_CODE_OK;
}
