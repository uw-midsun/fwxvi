/************************************************************************************************
 * @file   test_relays.c
 *
 * @brief  Test file for relays module
 *
 * @date   2025-10-24
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "gpio.h"
#include "rear_controller.h"
#include "relays.h"

static RearControllerStorage s_test_rear_storage;

static GpioAddress pos_relay_en = GPIO_REAR_CONTROLLER_POS_RELAY_ENABLE;
static GpioAddress pos_relay_sense = GPIO_REAR_CONTROLLER_POS_RELAY_SENSE;
static GpioAddress neg_relay_en = GPIO_REAR_CONTROLLER_NEG_RELAY_ENABLE;
static GpioAddress neg_relay_sense = GPIO_REAR_CONTROLLER_NEG_RELAY_SENSE;
static GpioAddress solar_relay_en = GPIO_REAR_CONTROLLER_SOLAR_RELAY_ENABLE;
static GpioAddress solar_relay_sense = GPIO_REAR_CONTROLLER_SOLAR_RELAY_SENSE;
static GpioAddress motor_relay_en = GPIO_REAR_CONTROLLER_MOTOR_RELAY_ENABLE;
static GpioAddress motor_relay_sense = GPIO_REAR_CONTROLLER_MOTOR_RELAY_SENSE;

void setup_test(void) {
  gpio_init();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, relays_init(&s_test_rear_storage));
}

void teardown_test(void) {}

TEST_IN_TASK
void test_relays_init_initializes_all_pins(void) {
  // After initialization, all enable pins should be LOW
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, gpio_get_state(&pos_relay_en));
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, gpio_get_state(&neg_relay_en));
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, gpio_get_state(&solar_relay_en));
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, gpio_get_state(&motor_relay_en));

  // Relay closed flags should be false
  TEST_ASSERT_FALSE(s_test_rear_storage.pos_relay_closed);
  TEST_ASSERT_FALSE(s_test_rear_storage.neg_relay_closed);
  TEST_ASSERT_FALSE(s_test_rear_storage.solar_relay_closed);
  TEST_ASSERT_FALSE(s_test_rear_storage.motor_relay_closed);
}

TEST_IN_TASK
void test_relays_reset_opens_all_relays(void) {
  // Pretend relays are closed
  s_test_rear_storage.pos_relay_closed = true;
  s_test_rear_storage.neg_relay_closed = true;
  s_test_rear_storage.solar_relay_closed = true;
  s_test_rear_storage.motor_relay_closed = true;

  // Call fault
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, relays_reset());

  // Verify all enables are low
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, gpio_get_state(&pos_relay_en));
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, gpio_get_state(&neg_relay_en));
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, gpio_get_state(&solar_relay_en));
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, gpio_get_state(&motor_relay_en));

  // Verify internal flags reset
  TEST_ASSERT_FALSE(s_test_rear_storage.pos_relay_closed);
  TEST_ASSERT_FALSE(s_test_rear_storage.neg_relay_closed);
  TEST_ASSERT_FALSE(s_test_rear_storage.solar_relay_closed);
  TEST_ASSERT_FALSE(s_test_rear_storage.motor_relay_closed);
}

TEST_IN_TASK
void test_relay_close_and_open_motor(void) {
  // Mock the sense pin to simulate successful close
  gpio_set_state(&motor_relay_sense, GPIO_STATE_HIGH);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, relays_close_motor());
  TEST_ASSERT_TRUE(s_test_rear_storage.motor_relay_closed);

  // Mock the sense pin to simulate successful open
  gpio_set_state(&motor_relay_sense, GPIO_STATE_LOW);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, relays_open_motor());
  TEST_ASSERT_FALSE(s_test_rear_storage.motor_relay_closed);
}

TEST_IN_TASK
void test_relay_close_and_open_solar(void) {
  gpio_set_state(&solar_relay_sense, GPIO_STATE_HIGH);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, relays_close_solar());
  TEST_ASSERT_TRUE(s_test_rear_storage.solar_relay_closed);

  gpio_set_state(&solar_relay_sense, GPIO_STATE_LOW);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, relays_open_solar());
  TEST_ASSERT_FALSE(s_test_rear_storage.solar_relay_closed);
}

TEST_IN_TASK
void test_relay_close_and_open_pos(void) {
  gpio_set_state(&pos_relay_sense, GPIO_STATE_HIGH);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, relays_close_pos());
  TEST_ASSERT_TRUE(s_test_rear_storage.pos_relay_closed);

  gpio_set_state(&pos_relay_sense, GPIO_STATE_LOW);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, relays_open_pos());
  TEST_ASSERT_FALSE(s_test_rear_storage.pos_relay_closed);
}

TEST_IN_TASK
void test_relay_close_and_open_neg(void) {
  gpio_set_state(&neg_relay_sense, GPIO_STATE_HIGH);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, relays_close_neg());
  TEST_ASSERT_TRUE(s_test_rear_storage.neg_relay_closed);

  gpio_set_state(&neg_relay_sense, GPIO_STATE_LOW);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, relays_open_neg());
  TEST_ASSERT_FALSE(s_test_rear_storage.neg_relay_closed);
}

TEST_IN_TASK
void test_relay_close_failure_returns_error(void) {
  // Make sure sense pin remains low when trying to close
  gpio_set_state(&pos_relay_sense, GPIO_STATE_LOW);
  TEST_ASSERT_EQUAL(STATUS_CODE_INTERNAL_ERROR, relays_close_pos());
}

TEST_IN_TASK
void test_relay_open_failure_returns_error(void) {
  // Force sense pin to stay high even after open
  gpio_set_state(&pos_relay_sense, GPIO_STATE_HIGH);
  TEST_ASSERT_EQUAL(STATUS_CODE_INTERNAL_ERROR, relays_open_pos());
}
