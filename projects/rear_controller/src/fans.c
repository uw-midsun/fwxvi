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

#define REAR_CONTROLLER_FAN_PERIOD_US 40U

/* Fan 1 (PB2) has no usable timer channel, so it is left floating and never driven */
static GpioAddress fans_2_address = GPIO_REAR_CONTROLLER_MOTOR_FAN_PWM_2;
static GpioAddress fans_pwr_1_address = GPIO_REAR_CONTROLLER_MOTOR_FAN_PWR_1;

static RearControllerStorage *rear_controller_storage;

StatusCode fans_run() {
  if (rear_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  /* Ramp fan 2 from idle to full linearly over the temp band, using the hotter of board/cell */
  int16_t max_temp = rear_controller_storage->max_board_temperature;
  if (rear_controller_storage->max_cell_temperature > max_temp) {
    max_temp = rear_controller_storage->max_cell_temperature;
  }

  uint16_t duty;
  if (max_temp <= REAR_CONTROLLER_FAN_RAMP_MIN_C) {
    duty = REAR_CONTROLLER_FAN_IDLE_DUTY_PCT;
  } else if (max_temp >= REAR_CONTROLLER_FAN_RAMP_MAX_C) {
    duty = REAR_CONTROLLER_FAN_MAX_DUTY_PCT;
  } else {
    duty = REAR_CONTROLLER_FAN_IDLE_DUTY_PCT + (uint16_t)((max_temp - REAR_CONTROLLER_FAN_RAMP_MIN_C) * (REAR_CONTROLLER_FAN_MAX_DUTY_PCT - REAR_CONTROLLER_FAN_IDLE_DUTY_PCT) /
                                                          (REAR_CONTROLLER_FAN_RAMP_MAX_C - REAR_CONTROLLER_FAN_RAMP_MIN_C));
  }

  status_ok_or_return(pwm_set_dc(PWM_TIMER_2, duty, PWM_CHANNEL_2, false));

  /* fan_pwr_1 is the dumb full-power enable; keep it off unless BPS is triggered */
  GpioState pwr_state = (rear_controller_storage->bps_fault_record.fault_code != 0U) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
  status_ok_or_return(gpio_set_state(&fans_pwr_1_address, pwr_state));

  return STATUS_CODE_OK;
}

StatusCode fans_init(RearControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  rear_controller_storage = storage;

  /* Fan 1 (PB2) is intentionally left floating - no valid timer channel on that pin */

  /* Fan 2 (PB3) is TIM2_CH2 (AF1) */
  status_ok_or_return(gpio_init_pin_af(&fans_2_address, GPIO_ALTFN_PUSH_PULL, GPIO_ALT1_TIM2));
  status_ok_or_return(pwm_init(PWM_TIMER_2, REAR_CONTROLLER_FAN_PERIOD_US));
  status_ok_or_return(pwm_set_dc(PWM_TIMER_2, 0U, PWM_CHANNEL_2, false));

  status_ok_or_return(gpio_init_pin(&fans_pwr_1_address, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW));

  return STATUS_CODE_OK;
}
