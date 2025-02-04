#pragma once

/************************************************************************************************
 * @file   pwm.h
 *
 * @brief  PWM Library Header file
 *
 * @date   2025-01-15
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "status.h"

typedef enum {
  PWM_TIMER_1 = 0,  // advanced control
  PWM_TIMER_2,      // general purpose
  PWM_TIMER_3,      // general purpose
  PWM_TIMER_4,      // general purpose
  PWM_TIMER_5,      // general purpose
  PWM_TIMER_6,      // basic
  NUM_PWM_TIMERS,
} PwmTimer;

typedef enum {
  PWM_CHANNEL_1 = 0,
  PWM_CHANNEL_2,
  PWM_CHANNEL_3,
  PWM_CHANNEL_4,
  PWM_CHANNEL_5,
  PWM_CHANNEL_6,
  NUM_PWM_CHANNELS  // max # of pwm channels
} PwmChannel;

// Initializes the PWM for a set timer with a specific frequency (Hz)
#define pwm_init_hz(timer, frequency) pwm_init((timer), 1000000 / (frequency))

/**
 * @brief   Initializes a specified PWM timer with a period
 * @param   timer PWM timer to configure
 * @param   period_ms Duration of one cycle in microseconds
 * @return  STATUS_CODE_OK if PWM timer initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode pwm_init(PwmTimer timer, uint16_t period_us);

/**
 * @brief   Gets the current period of a specified PWM timer in microseconds
 * @param   timer PWM timer to check
 * @return  STATUS_CODE_OK if period retrieval succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
uint16_t pwm_get_period(PwmTimer timer);

/**
 * @brief   Sets the pulse width of a specified channel of a PWM timer; High-res control
 * @param   timer PWM timer to configure
 * @param   pulse_width_us Pulse width in microseconds
 * @param   channel Channel of the PWM timer to configure
 * @param   n_channel_en Whether n channel is enabled or not
 * @return  STATUS_CODE_OK if PWM duty cycle configuration succeeded
 *          STATUS_CODE_UNINITIALIZED if PWM timer was not initialized
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode pwm_set_pulse(PwmTimer timer, uint16_t pulse_width_us, PwmChannel channel, bool n_channel_en);

/**
 * @brief   Sets the duty cycle of a specificed channel for a PWM timer
 * @param   timer PWM timer to configure
 * @param   dc Duty cycle in units of 1%
 * @param   channel Channel of the PWM timer to configure
 * @param   n_channel_en Whether n channel is enabled or not
 * @return  STATUS_CODE_OK if PWM duty cycle configuration succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode pwm_set_dc(PwmTimer timer, uint16_t dc, PwmChannel channel, bool n_channel_en);

/**
 * @brief   Gets the duty cycle of a specified channel for a PWM timer
 * @param   timer PWM timer to configure
 * @param   channel Channel of the PWM timer to configure
 * @return  STATUS_CODE_OK if duty cycle retrieval succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
uint16_t pwm_get_dc(PwmTimer timer, PwmChannel channel);