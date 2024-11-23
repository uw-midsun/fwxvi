#pragma once

#include "stm32l4xx_hal_tim.h"
#include "status.h"
#include "pwm_mcu.h"

#include <stdbool.h>
#include <stdint.h>




#define pwm_init_hz(timer, frequency) pwm_init((timer), 1000000 / (frequency))


StatusCode pwm_init(PwmTimer timer, uint16_t period_us);


uint16_t pwm_get_period(PwmTimer timer);


StatusCode pwm_set_pulse(PwmTimer timer, uint16_t pulse_width_us, uint8_t channel, bool n_channel_en);


StatusCode pwm_set_dc(PwmTimer timer, uint16_t dc, uint8_t channel, bool n_channel_en);


uint16_t pwm_get_dc(PwmTimer timer);
