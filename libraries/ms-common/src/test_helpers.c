/************************************************************************************************
 * @file   test_helpers.c
 *
 * @brief  Source code for the Test helpers
 *
 * @date   2025-07-17
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "semphr.h"

/* Intra-component Headers */
#include "test_helpers.h"

#define MAX_NUM_TESTS 10

static StaticSemaphore_t s_test_helpers_start_sem;
static SemaphoreHandle_t s_test_helpers_start_handle;

static StaticSemaphore_t s_test_helpers_end_sem;
static SemaphoreHandle_t s_test_helpers_end_handle;

static TickType_t TICKS_TO_DELAY = 100U;

void test_helpers_init_semphr(void) {
  s_test_helpers_start_handle = xSemaphoreCreateCountingStatic(MAX_NUM_TESTS, 10U, &s_test_helpers_start_sem);
  s_test_helpers_end_handle = xSemaphoreCreateCountingStatic(MAX_NUM_TESTS, 0U, &s_test_helpers_end_sem);
}

uint8_t test_helpers_start_get_semphr(void) {
  return uxSemaphoreGetCount(s_test_helpers_start_handle);
}

uint8_t test_helpers_end_get_semphr(void) {
  return uxSemaphoreGetCount(s_test_helpers_end_handle);
}

StatusCode test_helpers_start_give_semphr(void) {
  if (xSemaphoreGive(s_test_helpers_start_handle) == pdTRUE) {
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_INTERNAL_ERROR;
  }
}

StatusCode test_helpers_end_give_semphr(void) {
  if (xSemaphoreGive(s_test_helpers_end_handle) == pdTRUE) {
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_INTERNAL_ERROR;
  }
}

StatusCode test_helpers_start_take_semphr(void) {
  if (xSemaphoreTake(s_test_helpers_start_handle, TICKS_TO_DELAY) == pdTRUE) {
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_INTERNAL_ERROR;
  }
}

StatusCode test_helpers_end_take_semphr(void) {
  if (xSemaphoreTake(s_test_helpers_end_handle, TICKS_TO_DELAY) == pdTRUE) {
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_INTERNAL_ERROR;
  }
}
