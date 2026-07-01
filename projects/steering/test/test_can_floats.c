/************************************************************************************************
 * @file   test_can_float_encoding.c
 *
 * @brief  Test file for CAN float signal encoding and decoding
 *
 * @date   2026-06-10
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "can_codegen.h"
#include "rear_controller_getters.h"
#include "rear_controller_setters.h"

void setup_test(void) {}

void teardown_test(void) {}

TEST_PRE_TASK
void test_afe_voltage_round_trip(void) {
  float test_values[] = { 20000.0f, 35000.0f, 50000.0f, 42000.0f, 25000.0f };
  float max_error = (50000.0f - 20000.0f) / 253.0f;

  for (int i = 0; i < 5; i++) {
    set_AFE1_status_A_voltage_0(test_values[i]);
    g_rx_struct.AFE1_status_A_voltage_0 = g_tx_struct.AFE1_status_A_voltage_0;
    float result = get_AFE1_status_A_voltage_0();
    TEST_ASSERT_FLOAT_WITHIN(max_error, test_values[i], result);
  }
}

TEST_PRE_TASK
void test_afe_voltage_bound(void) {
  set_AFE1_status_A_voltage_0(20000.0f);
  TEST_ASSERT_EQUAL_UINT8(1, g_tx_struct.AFE1_status_A_voltage_0);

  set_AFE1_status_A_voltage_0(50000.0f);
  TEST_ASSERT_EQUAL_UINT8(254, g_tx_struct.AFE1_status_A_voltage_0);
}

TEST_PRE_TASK
void test_afe_voltage_sentinel(void) {
  set_AFE1_status_A_voltage_0(10000.0f);
  TEST_ASSERT_EQUAL_UINT8(0, g_tx_struct.AFE1_status_A_voltage_0);

  set_AFE1_status_A_voltage_0(60000.0f);
  TEST_ASSERT_EQUAL_UINT8(255, g_tx_struct.AFE1_status_A_voltage_0);
}

TEST_PRE_TASK
void test_afe_voltage_sentinel_getter(void) {
  g_rx_struct.AFE1_status_A_voltage_0 = 0;
  float below = get_AFE1_status_A_voltage_0();
  TEST_ASSERT_TRUE(below < 20000.0f);

  g_rx_struct.AFE1_status_A_voltage_0 = 255;
  float above = get_AFE1_status_A_voltage_0();
  TEST_ASSERT_TRUE(above > 50000.0f);
}