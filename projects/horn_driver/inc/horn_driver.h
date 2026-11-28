#pragma once

/************************************************************************************************
 * @file   horn_driver.h
 *
 * @brief  Header file for horn_driver
 *
 * @date   2026-10-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */

// libraries
#include "stm32l4xx_hal.h"
// macros
#define TIM1_CH1 8      // PA8 Q1
#define TIM1_CH1N 7     // PA7 Q3
#define TIM1_CH2 9      // PA9 Q2
#define TIM1_CH2N 0     // PB0 Q4
#define DEAD_TIME_US 5     // in us
#define DUTY_CYCLE_PER 0.5  // 50% duty cycle

/**
 * @defgroup horn_driver
 * @brief    horn_driver Firmware
 * @{
 */

typedef enum {
  ALL_OFF = 0,
  Q1_Q4_ON = 1,
  Q2_Q3_ON = 2,
} MosfetState;

StatusCode horn_driver_init();

StatusCode horn_driver_enable(bool is_enabled);
/** @} */