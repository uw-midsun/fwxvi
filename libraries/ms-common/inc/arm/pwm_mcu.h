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
  PWM_TIMER_2,
  PWM_TIMER_6,  // Usually used by i2c
  PWM_TIMER_7,
  PWM_TIMER_15,
  PWM_TIMER_16,
  NUM_PWM_TIMERS,
} PwmTimer;

typedef enum {
  PWM_CHANNEL_1 = 0,
  PWM_CHANNEL_2,
  NUM_PWM_CHANNELS,
} PwmChannel;
