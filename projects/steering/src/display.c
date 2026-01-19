/************************************************************************************************
 * @file   display.c
 *
 * @brief  Source file for display control
 *
 * @date   2025-07-22
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>

/* Inter-component Headers */
#include "gpio.h"
#include "pwm.h"

/* Intra-component Headers */
#include "display.h"
#include "steering_hw_defs.h"

static SteeringStorage *steering_storage = NULL;

/* Enable display when high */
static GpioAddress s_display_ctrl = STEERING_DISPLAY_CTRL;
static GpioAddress s_display_current_ctrl = STEERING_DISPLAY_CURRENT_CTRL;

StatusCode display_init(SteeringStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  steering_storage = storage;

  gpio_init_pin(&s_display_ctrl, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&s_display_current_ctrl, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

  return STATUS_CODE_OK;
}
