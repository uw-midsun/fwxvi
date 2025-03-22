/************************************************************************************************
 * @file   pwm.c
 *
 * @brief  PWM Library Source code
 *
 * @date   2025-01-13
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "log.h"
#include "pwm.h"
#include "status.h"

/**
 * @brief   PWM Timer details
 * @details Stores period and dc while tracking enabled (n) channels
 */
typedef struct PwmInfo {
  uint16_t period;
  bool initialized;
  uint16_t duty_cycles[NUM_PWM_CHANNELS];
  bool channel_en[NUM_PWM_CHANNELS];
  bool n_channel_en[NUM_PWM_CHANNELS];
} PwmInfo;

/** @brief  Number of channels associated to each PWM timer*/
static const uint8_t num_channels[NUM_PWM_TIMERS] = {
  6U, /**< TIM1 - 6 channels */
  4U, /**< TIM2 - 4 channels */
  2U, /**< TIM15 - 2 channels */
  1U  /**< TIM16 - 1 channel */
};

PwmInfo pwm[NUM_PWM_TIMERS];

StatusCode pwm_init(PwmTimer timer, uint16_t period_us) {
  if (timer >= NUM_PWM_TIMERS) {
    LOG_DEBUG("Invalid timer id");
    return STATUS_CODE_INVALID_ARGS;
  } else if (period_us == 0) {
    LOG_DEBUG("Period must be greater than 0");
    return STATUS_CODE_INVALID_ARGS;
  }

  pwm[timer].period = period_us;
  pwm[timer].initialized = true;

  return STATUS_CODE_OK;
}

uint16_t pwm_get_period(PwmTimer timer) {
  if (timer >= NUM_PWM_TIMERS) {
    LOG_DEBUG("Invalid timer id");
    return STATUS_CODE_INVALID_ARGS;
  }

  return pwm[timer].period;
}

StatusCode pwm_set_pulse(PwmTimer timer, uint16_t pulse_width_us, PwmChannel channel, bool n_channel_en) {
  if (timer >= NUM_PWM_TIMERS) {
    LOG_DEBUG("Invalid timer id");
    return STATUS_CODE_INVALID_ARGS;
  } else if (pwm[timer].period == 0) {
    LOG_DEBUG("Pwm must be initialized");
    return STATUS_CODE_UNINITIALIZED;
  } else if (pulse_width_us > pwm[timer].period) {
    LOG_DEBUG("Pulse width must be leq period");
    return STATUS_CODE_INVALID_ARGS;
  }

  (pwm[timer]).duty_cycles[channel] = (pulse_width_us * 100) / pwm[timer].period;

  return STATUS_CODE_OK;
}

StatusCode pwm_set_dc(PwmTimer timer, uint16_t dc, PwmChannel channel, bool n_channel_en) {
  if (timer >= NUM_PWM_TIMERS) {
    LOG_DEBUG("Invalid timer id");
    return STATUS_CODE_INVALID_ARGS;
  } else if (channel > num_channels[timer]) {
    LOG_DEBUG("Invalid channel number");
    return STATUS_CODE_INVALID_ARGS;
  } else if (dc > 100) {
    LOG_DEBUG("Duty Cycle must be leq 100 percent");
    return STATUS_CODE_INVALID_ARGS;
  }

  (pwm[timer]).duty_cycles[channel] = dc;
  (pwm[timer]).channel_en[channel] = true;
  (pwm[timer]).n_channel_en[channel] = true;

  return STATUS_CODE_OK;
}

uint16_t pwm_get_dc(PwmTimer timer, PwmChannel channel) {
  if (timer >= NUM_PWM_TIMERS) {
    LOG_DEBUG("Invalid timer id");
    return STATUS_CODE_INVALID_ARGS;
  } else if (channel > num_channels[timer]) {
    LOG_DEBUG("Invalid channel number");
    return STATUS_CODE_INVALID_ARGS;
  }

  return (pwm[timer]).duty_cycles[channel];
}
