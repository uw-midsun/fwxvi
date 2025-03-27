/************************************************************************************************
 * @file   thread_helpers.cc
 *
 * @brief  Source file defining the thread helpers
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include <time.h>

/* Intra-component Headers */
#include "thread_helpers.h"

void thread_sleep_s(unsigned int seconds) {
  struct timespec ts;
  ts.tv_sec = seconds;
  ts.tv_nsec = 0U;
  nanosleep(&ts, NULL);
}

void thread_sleep_ms(unsigned int milliseconds) {
  struct timespec ts;
  ts.tv_sec = milliseconds / 1000U;
  ts.tv_nsec = (milliseconds % 1000U) * 1000000U;
  nanosleep(&ts, NULL);
}
