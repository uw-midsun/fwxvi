/************************************************************************************************
 * @file    fota_timeout.c
 *
 * @brief   FOTA Timeout
 *
 * @date    2025-06-14
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "fota_timeout.h"

/* Intra-component Headers */
#include "stm32l4xx_hal.h"

bool fota_is_timed_out(void) {
  return HAL_GetTick() >= FOTA_TIMEOUT_MS;
}
