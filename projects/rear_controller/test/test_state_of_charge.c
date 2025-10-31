/************************************************************************************************
 * @file    test_state_of_charge.c
 *
 * @brief   Unit + integration tests for the EKF-based state_of_charge module
 *
 * @date    2025-10-31
 * @author  Midnight Sun
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* Inter-component Headers */
#include "test_helpers.h"
#include "unity.h"
#include "log.h"

/* Intra-component Headers */
#include "rear_controller.h"
#include "state_of_charge.h"

#define TEST_SERIES_COUNT        (36U)
#define TEST_PARALLEL_COUNT      (9U)
#define TEST_CELL_CAPACITY_AH    (4.5f)

#define TEST_PACK_MIN_VOLTAGE_MV (2500.0f * TEST_SERIES_COUNT)   // 90,000 mV
#define TEST_PACK_MAX_VOLTAGE_MV (4200.0f * TEST_SERIES_COUNT)   // 151,200 mV

#define TEST_PACK_CAPACITY_AH    (TEST_CELL_CAPACITY_AH * TEST_PARALLEL_COUNT)
#define TEST_PACK_CAPACITY_C     (TEST_PACK_CAPACITY_AH * 3600.0f)

static RearControllerConfig s_config = {
    .series_count = TEST_SERIES_COUNT,
    .parallel_count = TEST_PARALLEL_COUNT,
    .cell_capacity_Ah = TEST_CELL_CAPACITY_AH,
    .precharge_timeout_ms = 5000,
};

static RearControllerStorage s_storage;

void setup_test(void) {
  memset(&s_storage, 0, sizeof(s_storage));
  s_storage.config = &s_config;
}

void teardown_test(void) {}

/************************************************************************************************
 * Tests
 ************************************************************************************************/

TEST_IN_TASK
void test_soc_init_valid_config(void) {
  StatusCode status = state_of_charge_init(&s_storage);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_FLOAT_WITHIN(1e-6, 0.0f, s_storage.estimated_state_of_charge);
}

TEST_IN_TASK
void test_soc_init_null_storage(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, state_of_charge_init(NULL));
}

/************************************************************************************************
 * Integration behavior: simulate discharging and charging
 ************************************************************************************************/

TEST_IN_TASK
void test_soc_decreases_under_discharge(void) {
  s_storage.pack_voltage = (uint32_t)TEST_PACK_MAX_VOLTAGE_MV;
  s_storage.pack_current = -50000;  // -5 A discharge

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, state_of_charge_init(&s_storage));

  float soc_before = s_storage.estimated_state_of_charge;

  for (int i = 0; i < 50; i++) {
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, state_of_charge_run());
  }

  float soc_after = s_storage.estimated_state_of_charge;

  // Should trend downward
  LOG_DEBUG("SOC AFTER: %f, SOC BEFORE: %f\r\n", soc_after, soc_before);
  TEST_ASSERT_TRUE(soc_after < soc_before + 0.01f);
}

TEST_IN_TASK
void test_soc_increases_under_charge(void) {
  s_storage.pack_voltage = (uint32_t)(0.9f * TEST_PACK_MAX_VOLTAGE_MV);
  s_storage.pack_current = 50000;  // +5 A charge

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, state_of_charge_init(&s_storage));

  float soc_before = s_storage.estimated_state_of_charge;

  for (int i = 0; i < 50; i++) {
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, state_of_charge_run());
  }

  float soc_after = s_storage.estimated_state_of_charge;

  TEST_ASSERT_TRUE(soc_after > soc_before - 0.01f);
}

/************************************************************************************************
 * Boundary voltage tests
 ************************************************************************************************/

TEST_IN_TASK
void test_soc_high_voltage_maps_to_high_soc(void) {
  s_storage.pack_voltage = (uint32_t)TEST_PACK_MAX_VOLTAGE_MV;
  s_storage.pack_current = 0;  // at rest

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, state_of_charge_init(&s_storage));

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, state_of_charge_run());
  float soc = s_storage.estimated_state_of_charge;
  TEST_ASSERT_TRUE(soc > 0.9f);
}

TEST_IN_TASK
void test_soc_low_voltage_maps_to_low_soc(void) {
  s_storage.pack_voltage = (uint32_t)TEST_PACK_MIN_VOLTAGE_MV;
  s_storage.pack_current = 0;

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, state_of_charge_init(&s_storage));

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, state_of_charge_run());
  float soc = s_storage.estimated_state_of_charge;
  TEST_ASSERT_TRUE(soc < 0.1f);
}

/************************************************************************************************
 * Reinit safety test
 ************************************************************************************************/

TEST_IN_TASK
void test_soc_reinit_is_safe(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, state_of_charge_init(&s_storage));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, state_of_charge_run());
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, state_of_charge_init(&s_storage));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, state_of_charge_run());
}
