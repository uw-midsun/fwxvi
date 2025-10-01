/***********************************************************************************************
 * @file    test_thermistor.c
 *
 * @brief   Test file for thermistor module
 *
 * @date    2025-09-29
 * @author  Midnight Sun Team #24 - MSXVI
 ***********************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "thermistor.h"

void setup_test(void) {}

void teardown_test(void) {}

static uint16_t mock_voltage_reading = 0;

void test_thermistor(void) {
  mock_voltage_reading = 3000;
  TEST_ASSERT_EQUAL(15, calculate_board_thermistor_temperature(mock_voltage_reading));

  mock_voltage_reading = 2000;
  TEST_ASSERT_EQUAL(37, calculate_board_thermistor_temperature(mock_voltage_reading));
}
