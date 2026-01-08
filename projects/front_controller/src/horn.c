/************************************************************************************************
 * @file    horn.c
 *
 * @brief   Source file for horn manager
 *
 * @date    2025-11-17
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <math.h>

/* Inter-component Headers */
#include "adc.h"
#include "front_controller_getters.h"
#include "gpio.h"
#include "log.h"

/* Intra-component Headers */
#include "front_controller_hw_defs.h"
#include "horn.h"

static GpioAddress s_horn_gpio = FRONT_CONTROLLER_HORN_LS_ENABLE;

StatusCode horn_init() {
  gpio_init_pin(&s_horn_gpio, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  return STATUS_CODE_OK;
}
