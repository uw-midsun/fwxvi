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
#include "log.h"
#include "tasks.h"
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "aux_sense.h"
#include "bms_carrier.h"
#include "bms_hw_defs.h"

/** @brief  Voltage divider R2 */
#define R2_OHMS 10000
/** @brief  Voltage divider R1 */
#define R1_OHMS 47000

static BmsStorage mock_bms_storage = { .bms_config = { .series_count = NUM_SERIES_CELLS, .parallel_count = NUM_PARALLEL_CELLS, .pack_capacity = PACK_CAPACITY_MAH } };

float voltage_divider = (float)R2_OHMS / (float)(R2_OHMS + R1_OHMS);

void setup_test(void) {
  aux_sense_init(&mock_bms_storage);
}

void teardown_test(void) {}

TEST_IN_TASK
void test_aux_sense_0volts(void) {
  /* Set ADC reading to 0mV */
  uint32_t test_value = 0U;
  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_EQUAL_FLOAT(0, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_0p5volts(void) {
  /* Set ADC reading to 500mV */
  uint32_t test_value = 500U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 500, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_1volt(void) {
  /* Set ADC reading to 1000mV */
  uint32_t test_value = 1000U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 1000, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_1p5volts(void) {
  /* Set ADC reading to 1500mV */
  uint32_t test_value = 1500U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 1500U, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_2volts(void) {
  /* Set ADC reading to 2000mV */
  uint32_t test_value = 2000U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 2000, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_2p5volts(void) {
  /* Set ADC reading to 2500mV */
  uint32_t test_value = 2500U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 2500, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_3volts(void) {
  /* Set ADC reading to 3000mV */
  uint32_t test_value = 3000U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 3000, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_3p5volts(void) {
  /* Set ADC reading to 3500mV */
  uint32_t test_value = 3500U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 3500, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_4volts(void) {
  /* Set ADC reading to 4000mV */
  uint32_t test_value = 4000U * voltage_divider;
  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 4000, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_4p5volts(void) {
  /* Set ADC reading to 4500mV */
  uint32_t test_value = 4500U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 4500, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_5volts(void) {
  /* Set ADC reading to 5000mV */
  uint32_t test_value = 5000U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 5000, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_5p5volts(void) {
  /* Set ADC reading to 5500mV */
  uint32_t test_value = 5500U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 5500U, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_6volts(void) {
  /* Set ADC reading to 6000mV */
  uint32_t test_value = 6000U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 6000, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_6p5volts(void) {
  /* Set ADC reading to 6500mV */
  uint32_t test_value = 6500U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 6500, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_7volts(void) {
  /* Set ADC reading to 7000mV */
  uint32_t test_value = 7000U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 7000, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_7p5volts(void) {
  /* Set ADC reading to 7500mV */
  uint32_t test_value = 7500U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 7500, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_8volts(void) {
  /* Set ADC reading to 8000mV */
  uint32_t test_value = 8000U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 8000, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_8p5volts(void) {
  /* Set ADC reading to 8500mV */
  uint32_t test_value = 8500U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 8500, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_9volts(void) {
  /* Set ADC reading to 9000mV */
  uint32_t test_value = 9000U * voltage_divider;
  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 9000, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_9p5volts(void) {
  /* Set ADC reading to 9500mV */
  uint32_t test_value = 9500U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 9500, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_10volts(void) {
  /* Set ADC reading to 10000mV */
  uint32_t test_value = 10000U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 10000, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_10p5volts(void) {
  /* Set ADC reading to 10500mV */
  uint32_t test_value = 10500U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 10500U, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_11volts(void) {
  /* Set ADC reading to 11000mV */
  uint32_t test_value = 11000U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 11000, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_11p5volts(void) {
  /* Set ADC reading to 11500mV */
  uint32_t test_value = 11500U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 11500, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_12volts(void) {
  /* Set ADC reading to 12000mV */
  uint32_t test_value = 12000U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 12000, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}

TEST_IN_TASK
void test_aux_sense_12p5volts(void) {
  /* Set ADC reading to 12500mV */
  uint32_t test_value = 12500U * voltage_divider;

  adc_set_reading(&mock_bms_storage.aux_sense_storage->sense_adc, test_value);

  aux_sense_run();
  TEST_ASSERT_FLOAT_WITHIN(20, 12500, mock_bms_storage.aux_sense_storage->batt_voltage_mv);
}
