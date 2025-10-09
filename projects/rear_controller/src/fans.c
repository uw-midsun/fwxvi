/************************************************************************************************
 * @file    fans.c
 *
 * @brief   Fans source file
 *
 * @date    2025-10-01
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "gpio.h"
#include "pwm.h"
#include "status.h"

/* Intra-component Headers */
#include "fans.h"
#include "rear_controller.h"
#include "rear_controller_hw_defs.h"

#define REAR_CONTROLLER_FAN_PERIOD_US 40000U

static GpioAddress fans_address = REAR_CONTROLLER_MOTOR_FAN_PWM;

static RearControllerStorage *rear_controller_storage;

StatusCode fans_run() {
  if (rear_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (rear_controller_storage->max_board_temperature >= REAR_CONTROLLER_FAN_TEMP_THRESHOLD_C || rear_controller_storage->max_cell_temperature >= REAR_CONTROLLER_FAN_TEMP_THRESHOLD_C) {
    status_ok_or_return(pwm_set_dc(PWM_TIMER_3, 100U, PWM_CHANNEL_1, false));
  } else {
    status_ok_or_return(pwm_set_dc(PWM_TIMER_3, 0U, PWM_CHANNEL_1, false));
  }

  return STATUS_CODE_OK;
}

StatusCode fans_init(RearControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  rear_controller_storage = storage;

  status_ok_or_return(gpio_init_pin(&fans_address, GPIO_ALTFN_PUSH_PULL, GPIO_STATE_LOW));
  status_ok_or_return(pwm_init(PWM_TIMER_3, REAR_CONTROLLER_FAN_PERIOD_US));
  status_ok_or_return(pwm_set_dc(PWM_TIMER_3, 0U, PWM_CHANNEL_1, false));

  return STATUS_CODE_OK;
}
