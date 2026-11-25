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
#include "pwm.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_tim.h"
#include "stm32l4xx_ll_tim.h"
#include "stm32l4xx_hal_tim_ex.h"
// macros
#define PERIOD_MS 250      // 4kHz
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
static TIM_HandleTypeDef h_timer = { 0U };
StatusCode horn_driver_init();

StatusCode horn_driver_enable(bool is_enabled);

/** @} */