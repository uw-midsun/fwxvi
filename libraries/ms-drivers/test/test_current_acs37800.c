/************************************************************************************************
 * @file   test_current_acs37800.c
 *
 * @brief  Test file for ACS37800 current sensor x86 driver
 *
 * @date   2025-01-28
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <math.h>
#include <stdbool.h>

/* Inter-component Headers */
#include "log.h"
#include "unity.h"

/* Intra-component Headers */
#include "current_acs37800.h"

static ACS37800Storage s_storage;

void setup_test(void) {
  log_init();
  acs37800_init(&s_storage, I2C_PORT_1, 0x60);
}

void teardown_test(void) {}

// Init Test
void test_init_succeeds(void) {
  ACS37800Storage storage;
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, acs37800_init(&storage, I2C_PORT_1, 0x60));
}

void test_init_rejects_null(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, acs37800_init(NULL, I2C_PORT_1, 0x60));
}

void test_init_rejects_invalid_address(void) {
  ACS37800Storage storage;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, acs37800_init(&storage, I2C_PORT_1, 128));
}

// Current tests
void test_get_current_null_storage(void) {
  float current;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, acs37800_get_current(NULL, &current));
}

void test_current_round_trip_positive(void) {
  float current;
  acs37800_set_current(0.5f);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, acs37800_get_current(&s_storage, &current));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, current);
}

void test_current_round_trip_negative(void) {
  float current;
  acs37800_set_current(-0.25f);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, acs37800_get_current(&s_storage, &current));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, -0.25f, current);
}

// Voltage tests
void test_get_voltage_null_storage(void) {
  float voltage;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, acs37800_get_voltage(NULL, &voltage));
}

void test_voltage_round_trip(void) {
  float voltage;
  acs37800_set_voltage(0.3f);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, acs37800_get_voltage(&s_storage, &voltage));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.3f, voltage);
}

void test_current_voltage_independent(void) {
  float current, voltage;

  acs37800_set_current(0.1f);
  acs37800_set_voltage(0.2f);

  acs37800_get_current(&s_storage, &current);
  acs37800_get_voltage(&s_storage, &voltage);

  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.1f, current);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.2f, voltage);
}

// Power test
void test_get_power_null_storage(void) {
  float power;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, acs37800_get_active_power(NULL, &power));
}

void test_power_round_trip(void) {
  float power;
  acs37800_set_power(0.5f);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, acs37800_get_active_power(&s_storage, &power));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, power);
}

// Flag tests
void test_overcurrent_flag_set(void) {
  bool flag;
  acs37800_set_overcurrent_flag(true);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, acs37800_get_overcurrent_flag(&s_storage, &flag));
  TEST_ASSERT_TRUE(flag);
}

void test_overcurrent_flag_clear(void) {
  bool flag;
  acs37800_set_overcurrent_flag(false);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, acs37800_get_overcurrent_flag(&s_storage, &flag));
  TEST_ASSERT_FALSE(flag);
}

void test_overvoltage_flag_set(void) {
  bool flag;
  acs37800_set_overvoltage_flag(true);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, acs37800_get_overvoltage_flag(&s_storage, &flag));
  TEST_ASSERT_TRUE(flag);
}

void test_overvoltage_flag_clear(void) {
  bool flag;
  acs37800_set_overvoltage_flag(false);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, acs37800_get_overvoltage_flag(&s_storage, &flag));
  TEST_ASSERT_FALSE(flag);
}

void test_undervoltage_flag_set(void) {
  bool flag;
  acs37800_set_undervoltage_flag(true);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, acs37800_get_undervoltage_flag(&s_storage, &flag));
  TEST_ASSERT_TRUE(flag);
}

void test_undervoltage_flag_clear(void) {
  bool flag;
  acs37800_set_undervoltage_flag(false);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, acs37800_get_undervoltage_flag(&s_storage, &flag));
  TEST_ASSERT_FALSE(flag);
}

void test_reset_overcurrent_flag(void) {
  bool flag;
  acs37800_set_overcurrent_flag(true);
  acs37800_reset_overcurrent_flag(&s_storage);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, acs37800_get_overcurrent_flag(&s_storage, &flag));
  TEST_ASSERT_FALSE(flag);
}
