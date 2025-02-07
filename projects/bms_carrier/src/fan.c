/************************************************************************************************
 * @file   fan.c
 *
 * @brief  Source code for fans
 *
 * @date   2025-01-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "log.h"
#include "adc.h"
#include "pwm.h"

/* Intra-component Headers */
#include "fan.h"
#include "bms_hw_defs.h"

static struct FanSettings fan_settings ={
  .fan1_sense = BMS_FAN_SENSE_1_GPIO,
  .fan2_sense = BMS_FAN_SENSE_2_GPIO,
  .fan_pwm_ctrl = BMS_FAN_PWM_GPIO
};

static BmsStorage *bms_storage;

uint8_t calculate_fan_dc(uint16_t temperature) {
  /* Scale percent based on temp range, starting at 50% if fan is set on */
  uint8_t speed_perc = 100 * ((float)temperature - BMS_FAN_TEMP_LOWER_THRESHOLD) /
                       (float)(BMS_FAN_TEMP_UPPER_THRESHOLD - BMS_FAN_TEMP_LOWER_THRESHOLD);
  speed_perc = (speed_perc / 2U + BMS_FAN_BASE_DUTY_CYCLE);

  if (speed_perc > 100U) {
    return 100U;
  } else {
    return speed_perc;
  }
}

void update_fans(void) {
  if (bms_storage->ltc_afe_storage.max_temp >= BMS_FAN_TEMP_UPPER_THRESHOLD) {
    pwm_set_dc(PWM_TIMER_2, 100U, PWM_CHANNEL_1, false);
  } else if (bms_storage->ltc_afe_storage.max_temp <= BMS_FAN_TEMP_LOWER_THRESHOLD) {
    pwm_set_dc(PWM_TIMER_2, 0, PWM_CHANNEL_1, false);
  } else {
    pwm_set_dc(PWM_TIMER_2, calculate_fan_dc(bms_storage->ltc_afe_storage.max_temp), 1U, false);
  }
}

StatusCode fans_init(BmsStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  bms_storage = storage;
  bms_storage->fan_settings = &fan_settings;

  /* Initialize fan PWM */
  gpio_init_pin_af(&bms_storage->fan_settings->fan_pwm_ctrl, GPIO_ALTFN_OPEN_DRAIN, GPIO_ALT1_TIM2);
  pwm_init(PWM_TIMER_2, BMS_FAN_PERIOD_MS);
  pwm_set_pulse(PWM_TIMER_2, BMS_FAN_PERIOD_MS, PWM_CHANNEL_1, false);
  pwm_set_dc(PWM_TIMER_2, 0U, PWM_CHANNEL_1, false);

  /* Initialize fan sense */
  gpio_init_pin(&bms_storage->fan_settings->fan1_sense, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);
  gpio_init_pin(&bms_storage->fan_settings->fan2_sense, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);

  return STATUS_CODE_OK;
}