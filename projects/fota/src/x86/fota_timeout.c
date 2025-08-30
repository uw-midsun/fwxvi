/************************************************************************************************
 * @file    fota_timeout.c
 *
 * @brief   FOTA Timeout
 *
 * @date    2025-06-14
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#include <time.h>

/* Intra-component Headers */
#include "fota_timeout.h"

static uint32_t emulated_get_tick(void) {
  static struct timespec start_time = { 0U, 0U };
  struct timespec now;

  if (start_time.tv_sec == 0 && start_time.tv_nsec == 0U) {
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    return 0;
  }

  clock_gettime(CLOCK_MONOTONIC, &now);

  uint32_t elapsed_ms = (uint32_t)((now.tv_sec - start_time.tv_sec) * 1000U + (now.tv_nsec - start_time.tv_nsec) / 1000000U);

  return elapsed_ms;
}

bool fota_is_timed_out(void) {
  return emulated_get_tick() >= FOTA_TIMEOUT_MS;
}
