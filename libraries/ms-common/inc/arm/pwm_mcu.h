#pragma once

/************************************************************************************************
 * uart_mcu.h
 *
 * Header file for MCU specific PWM library
 *
 * Created: 2024-10-27
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

typedef enum {
  PWM_TIMER_1 = 0,
  PWM_TIMER_3,
  PWM_TIMER_4,  // Usually used by i2c
  PWM_TIMER_14,
  PWM_TIMER_15,
  PWM_TIMER_16,
  PWM_TIMER_17,
  NUM_PWM_TIMERS,
} PwmTimer;

typedef enum {
  PWM_CHANNEL_1 = 0,
  PWM_CHANNEL_2,
  NUM_PWM_CHANNELS,
} PwmChannel;
