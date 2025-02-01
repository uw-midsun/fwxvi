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
#include "test_helpers.h"
#include "unity.h"
#include "delay.h"
#include "tasks.h"
#include "log.h"

/* Intra-component Headers */
#include "bms_carrier.h"
#include "bms_hw_defs.h"
#include "state_of_charge.h"

static BmsStorage mock_bms_storage = { .bms_config = {  .series_count = NUM_SERIES_CELLS,
                                                        .parallel_count = NUM_PARALLEL_CELLS,
                                                        .pack_capacity = PACK_CAPACITY_MAH } };

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
