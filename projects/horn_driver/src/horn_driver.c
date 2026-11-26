/************************************************************************************************
 * @file   horn_driver.c
 *
 * @brief  Header file for horn_driver
 *
 * @date   2026-11-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "status.h"
#include "log.h"

/* Intra-component Headers */
#include "horn_driver.h"
#define PERIOD_MS 250      // 4kHz
// macros
static TIM_HandleTypeDef h_timer = { 0U };

StatusCode horn_driver_init() {
  pwm_init(PWM_TIMER_1, PERIOD_MS);
  if (HAL_TIM_Base_Init(&h_timer) != HAL_OK) {
    LOG_DEBUG("Error in HAL init\r\n");
  }
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode horn_driver_enable(bool is_enabled) {
  return STATUS_CODE_UNIMPLEMENTED;
}

/** @} */