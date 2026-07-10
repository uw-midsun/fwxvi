/************************************************************************************************
 * @file   mcu.c
 *
 * @brief  Source code for MCU intialization
 *
 * @date   2024-10-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdio.h>
#include <string.h>

/* Inter-component Headers */
#include "gpio.h"
#include "mcu.h"
#include "retarget.h"

/* Intra-component Headers */
#include "stm32l4xx_hal.h"

StatusCode mcu_init(void) {
  if (HAL_Init() != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  gpio_init();
  retarget_init();

  return mcu_clock_init();
}
