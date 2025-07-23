/************************************************************************************************
 * @file   test_state_of_charge.c
 *
 * @brief  Test file for state of charge
 *
 * @date   2025-01-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "delay.h"
#include "log.h"
#include "tasks.h"
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "bms_carrier.h"
#include "bms_hw_defs.h"
#include "state_of_charge.h"

static BmsStorage mock_bms_storage = { .bms_config = { .series_count = NUM_SERIES_CELLS, .parallel_count = NUM_PARALLEL_CELLS, .pack_capacity = PACK_CAPACITY_MAH } };

static TickType_t mock_time = 1;

TickType_t TEST_MOCK(xTaskGetTickCount)(void) {
  return mock_time;
}

void setup_test(void) {
  state_of_charge_init(&mock_bms_storage);
}

void teardown_test(void) {}

TEST_IN_TASK
void test_coulomb_counting_zero_current(void) {
  mock_bms_storage.pack_current = 0;
  mock_bms_storage.pack_voltage = 35000;
  set_averaged_soc(0.5f);

  coulomb_counting_soc();

  TEST_ASSERT_EQUAL_FLOAT(0.5f, get_i_soc());
}

TEST_IN_TASK
void test_coulomb_counting_constant_current(void) {
  /* Pack is drawing 100 Amps... I hope this never happens */
  mock_bms_storage.pack_current = -100000;
  set_last_current(-100000);
  set_averaged_soc(0.5f);

  mock_time = 0;
  set_last_time(xTaskGetTickCount());
  /* 1 Minute */
  mock_time = 60000;
  coulomb_counting_soc();

  float expected_soc = 0.5f + (0.5f * (-100000 - 100000) * (60.0f / 3600.0f)) / PACK_CAPACITY_MAH;
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, expected_soc, get_i_soc());

  set_averaged_soc(expected_soc);
  set_last_time(xTaskGetTickCount());
  /* 2 Minutes */
  mock_time = 120000;
  coulomb_counting_soc();

  expected_soc = expected_soc + (0.5f * (-100000 - 100000) * (60.0f / 3600.0f)) / PACK_CAPACITY_MAH;
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, expected_soc, get_i_soc());

  set_averaged_soc(expected_soc);
  set_last_time(xTaskGetTickCount());
  /* 3 Minutes */
  mock_time = 180000;
  coulomb_counting_soc();

  expected_soc = expected_soc + (0.5f * (-100000 - 100000) * (60.0f / 3600.0f)) / PACK_CAPACITY_MAH;
  LOG_DEBUG("Expected SOC %d\n", (int)(expected_soc * 100000.0f));
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, expected_soc, get_i_soc());
}

TEST_IN_TASK
void test_coulomb_counting_dynamic_current(void) {
  /* Pack is discharging at 200 Amps */
  mock_bms_storage.pack_current = -200000;
  set_last_current(-100000);
  set_averaged_soc(0.5f);

  mock_time = 0;
  set_last_time(xTaskGetTickCount());
  /* 1 Minute */
  mock_time = 60000;
  coulomb_counting_soc();

  float expected_soc = 0.5f + (0.5f * (-100000 - 200000) * (60.0f / 3600.0f)) / PACK_CAPACITY_MAH;
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, expected_soc, get_i_soc());

  set_averaged_soc(expected_soc);
  set_last_time(xTaskGetTickCount());

  set_last_current(-200000);
  /* Pack is discharging at 400 Amps */
  mock_bms_storage.pack_current = -400000;
  /* 2 Minutes */
  mock_time = 120000;
  coulomb_counting_soc();

  expected_soc = expected_soc + (0.5f * (-200000 - 400000) * (60.0f / 3600.0f)) / PACK_CAPACITY_MAH;
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, expected_soc, get_i_soc());

  set_averaged_soc(expected_soc);
  set_last_time(xTaskGetTickCount());

  set_last_current(-400000);
  /* Pack is charging at 300 Amps */
  mock_bms_storage.pack_current = 300000;
  /* 3 Minutes */
  mock_time = 180000;
  coulomb_counting_soc();

  expected_soc = expected_soc + (0.5f * (-400000 + 300000) * (60.0f / 3600.0f)) / PACK_CAPACITY_MAH;
  LOG_DEBUG("Expected SOC %d\n", (int)(expected_soc * 100000.0f));
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, expected_soc, get_i_soc());
}

TEST_IN_TASK
void test_initial_state_of_charge(void) {
  mock_bms_storage.pack_current = 0;
  mock_bms_storage.pack_voltage = 3500;
  state_of_charge_init(&mock_bms_storage);

  /* Check if the initial SOC is set based on the OCV voltage */
  float expected_soc = 0;
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, expected_soc, get_v_soc());
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, expected_soc, get_i_soc());
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, expected_soc, get_averaged_soc());
}

TEST_IN_TASK
void test_ramp_voltage_weight_high_soc(void) {
  set_averaged_soc(80.0f);
  ramp_voltage_weight();
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.20f + ((0.6f / 30.0f) * (80.0f - 70.0f)), get_voltage_weight());
}

TEST_IN_TASK
void test_ramp_voltage_weight_low_soc(void) {
  set_averaged_soc(20.0f);
  ramp_voltage_weight();
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.20f + ((0.6f / 30.0f) * (30.0f - 20.0f)), get_voltage_weight());
}

TEST_IN_TASK
void test_ramp_voltage_weight_normal_soc(void) {
  set_averaged_soc(50.0f);
  ramp_voltage_weight();
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.20f, get_voltage_weight());
}

TEST_IN_TASK
void test_ocv_voltage_soc_100_percent() {
  /* 152.0V */
  mock_bms_storage.pack_voltage = 152000;
  ocv_voltage_soc();
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, get_v_soc());
}

TEST_IN_TASK
void test_ocv_voltage_soc_80_percent() {
  /* 136.8V */
  mock_bms_storage.pack_voltage = 136800;
  ocv_voltage_soc();
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 80.0f, get_v_soc());
}

TEST_IN_TASK
void test_ocv_voltage_soc_50_percent() {
  /* 128.16V */
  mock_bms_storage.pack_voltage = 128160;
  ocv_voltage_soc();
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 50.0f, get_v_soc());
}

TEST_IN_TASK
void test_ocv_voltage_soc_30_percent() {
  /* 122.4V */
  mock_bms_storage.pack_voltage = 122400;
  ocv_voltage_soc();
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 30.0f, get_v_soc());
}

TEST_IN_TASK
void test_ocv_voltage_soc_0_percent() {
  /* 100V */
  mock_bms_storage.pack_voltage = 100000;
  ocv_voltage_soc();
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, get_v_soc());
}
