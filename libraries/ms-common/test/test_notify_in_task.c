/************************************************************************************************
 * @file   test_notify_in_task.c
 *
 * @brief  Test file for Notify library in tasks
 *
 * @date   2025-07-17
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "delay.h"
#include "log.h"
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "notify.h"

void setup_test(void) {
  log_init();
  test_helpers_init_semphr();
}

void teardown_test(void) {}

#define NUM_TEST_EVENTS 7
static Event s_notify_events[NUM_TEST_EVENTS] = { 0, 1, 2, 4, 8, 16, 31 };

TASK(receive_task, TASK_MIN_STACK_SIZE) {
  uint32_t notification;
  Event e;

  // First notify call, nothing should have arrived
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, notify_get(&notification));
  TEST_ASSERT_EQUAL(0, notification);
  test_helpers_end_give_semphr();

  // Second notify receive will receive first sent notification
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_EQUAL(event_from_notification(&notification, &e), STATUS_CODE_INCOMPLETE);
  TEST_ASSERT_EQUAL(s_notify_events[0], e);
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, notify_get(&notification));
  test_helpers_end_give_semphr();

  // Wait for notifications to stack up, then iterate through received
  delay_ms(1);
  uint8_t i = 0;
  while (event_from_notification(&notification, &e)) {
    TEST_ASSERT_EQUAL(s_notify_events[i], e);
    i++;
  }
  test_helpers_end_give_semphr();

  while (1) {
  }
}

TEST_IN_TASK
void test_notifications() {
  tasks_init_task(receive_task, TASK_PRIORITY(1), NULL);

  // Delay, then send first message
  test_helpers_end_take_semphr();
  TEST_ASSERT_OK(notify(receive_task, s_notify_events[0]));

  // Allow receive to catch up
  test_helpers_end_take_semphr();

  // Send all above notification in one fell swoop
  for (uint8_t i = 0; i < NUM_TEST_EVENTS; i++) {
    TEST_ASSERT_OK(notify(receive_task, s_notify_events[i]));
  }
  test_helpers_end_take_semphr();
}
