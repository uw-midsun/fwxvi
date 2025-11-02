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

<<<<<<< HEAD
static uint16_t mock_voltage_reading = 0;

void test_thermistor(void) {
  mock_voltage_reading = 3000;
  TEST_ASSERT_EQUAL(15, calculate_board_thermistor_temperature(mock_voltage_reading));

  mock_voltage_reading = 2000;
  TEST_ASSERT_EQUAL(37, calculate_board_thermistor_temperature(mock_voltage_reading));
=======
TEST_IN_TASK
void test_thermistor_voltage_to_temperature_basic(void) {
  /* High voltage should correspond to low temperature */
  uint16_t voltage_mv = 3000U;
  uint16_t expected_temp_c = 15U;
  TEST_ASSERT_EQUAL(expected_temp_c, calculate_board_thermistor_temperature(voltage_mv));

  /* Lower voltage should correspond to higher temperature */
  voltage_mv = 2000U;
  expected_temp_c = 37U;
  TEST_ASSERT_EQUAL(expected_temp_c, calculate_board_thermistor_temperature(voltage_mv));
}

TEST_IN_TASK
void test_thermistor_interpolation_behavior(void) {
  /* Readings between two table points should interpolate smoothly */
  uint16_t lower_voltage = 2200U;
  uint16_t upper_voltage = 2300U;

  uint16_t lower_temp = calculate_board_thermistor_temperature(lower_voltage);
  uint16_t upper_temp = calculate_board_thermistor_temperature(upper_voltage);

  /* As voltage increases (resistance increases), temperature should decrease */
  TEST_ASSERT_TRUE(upper_temp < lower_temp);
}

TEST_IN_TASK
void test_thermistor_out_of_range_input(void) {
  /* Zero voltage - unrealistic but should not crash or overflow */
  uint16_t voltage_mv = 0U;
  uint16_t temp_c = calculate_board_thermistor_temperature(voltage_mv);
  TEST_ASSERT_TRUE(temp_c <= 100U);

  /* Voltage equal to supply (5000 mV) - edge of division by zero */
  voltage_mv = 5000U;
  temp_c = calculate_board_thermistor_temperature(voltage_mv);
  TEST_ASSERT_TRUE(temp_c <= 100U);
>>>>>>> b7b2ab99daee8e52f10ea4d81c9b09d5dec1b333
}
