/************************************************************************************************
 * @file   test_drive_button_leds.c
 *
 * @brief  Test file for Steering Drive Button LEDs
 *
 * @date   2025-08-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "test_helpers.h"
#include "unity.h"
#include "log.h"

/* Intra-component Headers */
#include "drive_button_leds.h"

void setup_test(void) {}

void teardown_test(void) {}

void test_example(void) {
  TEST_ASSERT_TRUE(true);
}
TEST_IN_TASK
void test_main(void) {

  drive_button_led_init();

  drive_button_led_set_color(0, 1, 2, 4);
  drive_button_led_set_color(1, 255, 255, 255);
  drive_button_led_set_color(2, 4, 2, 1);

  drive_button_led_update();

  LOG_DEBUG("DMA Buffer Contents:\n");
  for (int i = 0; i < DMA_BUF_LEN; i++) {
      LOG_DEBUG("%u ", DMA_BUF[i]);
  }
  LOG_DEBUG("\n");

}