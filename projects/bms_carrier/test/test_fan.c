/************************************************************************************************
 * @file   test_aux_sense.c
 *
 * @brief  Test file for aux sense
 *
 * @date   2025-01-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "adc.h"
#include "delay.h"
#include "global_enums.h"
#include "log.h"
#include "tasks.h"
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "bms_carrier.h"
#include "bms_hw_defs.h"
#include "fan.h"
#include "fault_bps.h"

/** @brief  Voltage divider R2 */
#define R2_OHMS 10000
/** @brief  Voltage divider R1 */
#define R1_OHMS 47000

static BmsStorage mock_bms_storage = { .bms_config = { .series_count = NUM_SERIES_CELLS, .parallel_count = NUM_PARALLEL_CELLS, .pack_capacity = PACK_CAPACITY_MAH } };
void setup_test(void) {
  fans_init(&mock_bms_storage);
}

void teardown_test(void) {}

TEST_IN_TASK
void test_fan_state_lower_thresh_40c(void) {
  mock_bms_storage.max_temperature = BMS_FAN_TEMP_LOWER_THRESHOLD;

  uint8_t test_fan_dc = calculate_fan_dc(mock_bms_storage.max_temperature);

  TEST_ASSERT_EQUAL(test_fan_dc, BMS_FAN_BASE_DUTY_CYCLE);
}

TEST_IN_TASK
void test_fan_state_upper_thresh_50c(void) {
  mock_bms_storage.max_temperature = BMS_FAN_TEMP_UPPER_THRESHOLD;

  uint8_t test_fan_dc = calculate_fan_dc(mock_bms_storage.max_temperature);

  TEST_ASSERT_EQUAL(test_fan_dc, 100U);
}

TEST_IN_TASK
void test_fan_state_42c(void) {
  mock_bms_storage.max_temperature = 42U;

  uint8_t test_fan_dc = calculate_fan_dc(mock_bms_storage.max_temperature);
  uint8_t expected_fan_dc = 100 * ((float)42U - BMS_FAN_TEMP_LOWER_THRESHOLD) / (float)(BMS_FAN_TEMP_UPPER_THRESHOLD - BMS_FAN_TEMP_LOWER_THRESHOLD);

  expected_fan_dc = (expected_fan_dc / 2U + BMS_FAN_BASE_DUTY_CYCLE);

  TEST_ASSERT_EQUAL(test_fan_dc, expected_fan_dc);
}

TEST_IN_TASK
void test_fan_state_44c(void) {
  mock_bms_storage.max_temperature = 44U;

  uint8_t test_fan_dc = calculate_fan_dc(mock_bms_storage.max_temperature);
  uint8_t expected_fan_dc = 100 * ((float)44U - BMS_FAN_TEMP_LOWER_THRESHOLD) / (float)(BMS_FAN_TEMP_UPPER_THRESHOLD - BMS_FAN_TEMP_LOWER_THRESHOLD);

  expected_fan_dc = (expected_fan_dc / 2U + BMS_FAN_BASE_DUTY_CYCLE);

  TEST_ASSERT_EQUAL(test_fan_dc, expected_fan_dc);
}

TEST_IN_TASK
void test_fan_state_45c(void) {
  mock_bms_storage.max_temperature = 45U;

  uint8_t test_fan_dc = calculate_fan_dc(mock_bms_storage.max_temperature);
  uint8_t expected_fan_dc = 100 * ((float)45U - BMS_FAN_TEMP_LOWER_THRESHOLD) / (float)(BMS_FAN_TEMP_UPPER_THRESHOLD - BMS_FAN_TEMP_LOWER_THRESHOLD);

  expected_fan_dc = (expected_fan_dc / 2U + BMS_FAN_BASE_DUTY_CYCLE);

  TEST_ASSERT_EQUAL(test_fan_dc, expected_fan_dc);
}

TEST_IN_TASK
void test_fan_state_46c(void) {
  mock_bms_storage.max_temperature = 46U;

  uint8_t test_fan_dc = calculate_fan_dc(mock_bms_storage.max_temperature);
  uint8_t expected_fan_dc = 100 * ((float)46U - BMS_FAN_TEMP_LOWER_THRESHOLD) / (float)(BMS_FAN_TEMP_UPPER_THRESHOLD - BMS_FAN_TEMP_LOWER_THRESHOLD);

  expected_fan_dc = (expected_fan_dc / 2U + BMS_FAN_BASE_DUTY_CYCLE);

  TEST_ASSERT_EQUAL(test_fan_dc, expected_fan_dc);
}

TEST_IN_TASK
void test_fan_state_48c(void) {
  mock_bms_storage.max_temperature = 48U;

  uint8_t test_fan_dc = calculate_fan_dc(mock_bms_storage.max_temperature);
  uint8_t expected_fan_dc = 100 * ((float)48U - BMS_FAN_TEMP_LOWER_THRESHOLD) / (float)(BMS_FAN_TEMP_UPPER_THRESHOLD - BMS_FAN_TEMP_LOWER_THRESHOLD);

  expected_fan_dc = (expected_fan_dc / 2U + BMS_FAN_BASE_DUTY_CYCLE);

  TEST_ASSERT_EQUAL(test_fan_dc, expected_fan_dc);
}
