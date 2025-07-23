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
#include "fault_bps.h"
#include "relays.h"

/** @brief  Voltage divider R2 */
#define R2_OHMS 10000
/** @brief  Voltage divider R1 */
#define R1_OHMS 47000

static BmsStorage mock_bms_storage = { .bms_config = { .series_count = NUM_SERIES_CELLS, .parallel_count = NUM_PARALLEL_CELLS, .pack_capacity = PACK_CAPACITY_MAH } };
void setup_test(void) {
  fault_bps_init(&mock_bms_storage);
  relays_init(&mock_bms_storage);
}

void teardown_test(void) {}

TEST_IN_TASK
void test_fault_bps_overvoltage(void) {
  fault_bps_set(BMS_FAULT_OVERVOLTAGE);

  uint16_t test_fault_bps_priority = (1U << 15U);
  TEST_ASSERT_EQUAL(mock_bms_storage.fault_bps_storage->fault_bitset & test_fault_bps_priority, test_fault_bps_priority);

  uint16_t test_fault_bps = fault_bps_get();
  test_fault_bps &= ~(BMS_FAULT_KILLSWITCH_MASK);
  TEST_ASSERT_EQUAL((1U << BMS_FAULT_OVERVOLTAGE), test_fault_bps);

  fault_bps_clear(BMS_FAULT_OVERVOLTAGE);
}

TEST_IN_TASK
void test_fault_bps_unbalance(void) {
  fault_bps_set(BMS_FAULT_UNBALANCE);

  uint16_t test_fault_bps_priority = (1U << 15U);
  TEST_ASSERT_EQUAL(mock_bms_storage.fault_bps_storage->fault_bitset & test_fault_bps_priority, test_fault_bps_priority);

  uint16_t test_fault_bps = fault_bps_get();
  test_fault_bps &= ~(BMS_FAULT_KILLSWITCH_MASK);
  TEST_ASSERT_EQUAL((1U << BMS_FAULT_UNBALANCE), test_fault_bps);

  fault_bps_clear(BMS_FAULT_UNBALANCE);
}

TEST_IN_TASK
void test_fault_bps_overtemp_ambient(void) {
  fault_bps_set(BMS_FAULT_OVERTEMP_AMBIENT);

  uint16_t test_fault_bps_priority = (1U << 15U);
  TEST_ASSERT_EQUAL(mock_bms_storage.fault_bps_storage->fault_bitset & test_fault_bps_priority, test_fault_bps_priority);

  uint16_t test_fault_bps = fault_bps_get();
  test_fault_bps &= ~(BMS_FAULT_KILLSWITCH_MASK);
  TEST_ASSERT_EQUAL((1U << BMS_FAULT_OVERTEMP_AMBIENT), test_fault_bps);

  fault_bps_clear(BMS_FAULT_OVERTEMP_AMBIENT);
}

TEST_IN_TASK
void test_fault_bps_comms_loss_afe(void) {
  fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);

  uint16_t test_fault_bps_priority = (1U << 14U);
  TEST_ASSERT_EQUAL(mock_bms_storage.fault_bps_storage->fault_bitset & test_fault_bps_priority, test_fault_bps_priority);

  uint16_t test_fault_bps = fault_bps_get();
  test_fault_bps &= ~(BMS_FAULT_KILLSWITCH_MASK);
  TEST_ASSERT_EQUAL((1U << BMS_FAULT_COMMS_LOSS_AFE), test_fault_bps);

  fault_bps_clear(BMS_FAULT_COMMS_LOSS_AFE);
}

TEST_IN_TASK
void test_fault_bps_comms_loss_curr_sense(void) {
  fault_bps_set(BMS_FAULT_COMMS_LOSS_CURR_SENSE);

  uint16_t test_fault_bps_priority = (1U << 14U);
  TEST_ASSERT_EQUAL(mock_bms_storage.fault_bps_storage->fault_bitset & test_fault_bps_priority, test_fault_bps_priority);

  uint16_t test_fault_bps = fault_bps_get();
  test_fault_bps &= ~(BMS_FAULT_KILLSWITCH_MASK);
  TEST_ASSERT_EQUAL((1U << BMS_FAULT_COMMS_LOSS_CURR_SENSE), test_fault_bps);

  fault_bps_clear(BMS_FAULT_COMMS_LOSS_CURR_SENSE);
}

TEST_IN_TASK
void test_fault_bps_overcurrent(void) {
  fault_bps_set(BMS_FAULT_OVERCURRENT);

  uint16_t test_fault_bps_priority = (1U << 14U);
  TEST_ASSERT_EQUAL(mock_bms_storage.fault_bps_storage->fault_bitset & test_fault_bps_priority, test_fault_bps_priority);

  uint16_t test_fault_bps = fault_bps_get();
  test_fault_bps &= ~(BMS_FAULT_KILLSWITCH_MASK);
  TEST_ASSERT_EQUAL((1U << BMS_FAULT_OVERCURRENT), test_fault_bps);

  fault_bps_clear(BMS_FAULT_OVERCURRENT);
}

TEST_IN_TASK
void test_fault_bps_undervoltage(void) {
  fault_bps_set(BMS_FAULT_UNDERVOLTAGE);

  uint16_t test_fault_bps_priority = (1U << 14U);
  TEST_ASSERT_EQUAL(mock_bms_storage.fault_bps_storage->fault_bitset & test_fault_bps_priority, test_fault_bps_priority);

  uint16_t test_fault_bps = fault_bps_get();
  test_fault_bps &= ~(BMS_FAULT_KILLSWITCH_MASK);
  TEST_ASSERT_EQUAL((1U << BMS_FAULT_UNDERVOLTAGE), test_fault_bps);

  fault_bps_clear(BMS_FAULT_UNDERVOLTAGE);
}

TEST_IN_TASK
void test_fault_bps_relay_close_failed(void) {
  fault_bps_set(BMS_FAULT_RELAY_CLOSE_FAILED);

  uint16_t test_fault_bps_priority = (1U << 14U);
  TEST_ASSERT_EQUAL(mock_bms_storage.fault_bps_storage->fault_bitset & test_fault_bps_priority, test_fault_bps_priority);

  uint16_t test_fault_bps = fault_bps_get();
  test_fault_bps &= ~(BMS_FAULT_KILLSWITCH_MASK);
  TEST_ASSERT_EQUAL((1U << BMS_FAULT_RELAY_CLOSE_FAILED), test_fault_bps);

  fault_bps_clear(BMS_FAULT_RELAY_CLOSE_FAILED);
}

TEST_IN_TASK
void test_fault_bps_disconnected(void) {
  fault_bps_set(BMS_FAULT_DISCONNECTED);

  uint16_t test_fault_bps_priority = (1U << 14U);
  TEST_ASSERT_EQUAL(mock_bms_storage.fault_bps_storage->fault_bitset & test_fault_bps_priority, test_fault_bps_priority);

  uint16_t test_fault_bps = fault_bps_get();
  test_fault_bps &= ~(BMS_FAULT_KILLSWITCH_MASK);
  TEST_ASSERT_EQUAL((1U << BMS_FAULT_DISCONNECTED), test_fault_bps);

  fault_bps_clear(BMS_FAULT_DISCONNECTED);
}
