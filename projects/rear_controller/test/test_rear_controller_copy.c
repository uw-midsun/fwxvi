/************************************************************************************************
 * @file   test_rear_controller.c
 *
 * @brief  Test file for rear_controller and its state machine
 *
 * @date   2025-10-20
 * @author Midnight Sun Team #24 - MSXVI (revised by hungwn2/@copilot)
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* Inter-component Headers */
#include "delay.h"
#include "log.h"
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "rear_controller.h"
#include "rear_controller_hw_defs.h"
#include "rear_controller_state_manager.h"
#include "relays.h"

static RearControllerStorage s_test_storage;
static RearControllerConfig s_test_config;

static GpioState g_motor_enable_state = GPIO_STATE_LOW;
static GpioState g_solar_enable_state = GPIO_STATE_LOW;
static GpioState g_pos_enable_state = GPIO_STATE_LOW;
static GpioState g_neg_enable_state = GPIO_STATE_LOW;
static GpioState g_killswitch_state = GPIO_STATE_HIGH;  // Default to not active

// Mock for delay_ms
void delay_ms(uint32_t ms) {
  // No-op
}

// Helper for comparing GpioAddress
static bool prv_addr_eq(const GpioAddress *a, const GpioAddress *b) {
  return a->port == b->port && a->pin == b->pin;
}

StatusCode gpio_set_state(const GpioAddress *addr, GpioState state) {
  GpioAddress motor_en = REAR_CONTROLLER_MOTOR_RELAY_ENABLE_GPIO;
  if (prv_addr_eq(addr, &motor_en)) {
    g_motor_enable_state = state;
    return STATUS_CODE_OK;
  }
  GpioAddress solar_en = REAR_CONTROLLER_SOLAR_RELAY_ENABLE_GPIO;
  if (prv_addr_eq(addr, &solar_en)) {
    g_solar_enable_state = state;
    return STATUS_CODE_OK;
  }
  GpioAddress pos_en = REAR_CONTROLLER_POS_RELAY_ENABLE_GPIO;
  if (prv_addr_eq(addr, &pos_en)) {
    g_pos_enable_state = state;
    return STATUS_CODE_OK;
  }
  GpioAddress neg_en = REAR_CONTROLLER_NEG_RELAY_ENABLE_GPIO;
  if (prv_addr_eq(addr, &neg_en)) {
    g_neg_enable_state = state;
    return STATUS_CODE_OK;
  }
  return STATUS_CODE_OK;
}

GpioState gpio_get_state(const GpioAddress *addr) {
  GpioAddress motor_sense = REAR_CONTROLLER_MOTOR_RELAY_SENSE_GPIO;
  if (prv_addr_eq(addr, &motor_sense)) {
    return g_motor_enable_state;
  }
  GpioAddress solar_sense = REAR_CONTROLLER_SOLAR_RELAY_SENSE_GPIO;
  if (prv_addr_eq(addr, &solar_sense)) {
    return g_solar_enable_state;
  }
  GpioAddress pos_sense = REAR_CONTROLLER_POS_RELAY_SENSE_GPIO;
  if (prv_addr_eq(addr, &pos_sense)) {
    return g_pos_enable_state;
  }
  GpioAddress neg_sense = REAR_CONTROLLER_NEG_RELAY_SENSE_GPIO;
  if (prv_addr_eq(addr, &neg_sense)) {
    return g_neg_enable_state;
  }
  GpioAddress killswitch = REAR_CONTROLLER_KILLSWITCH_MONITOR_GPIO;
  if (prv_addr_eq(addr, &killswitch)) {
    return g_killswitch_state;
  }
  return GPIO_STATE_LOW;
}

StatusCode gpio_init_pin(const GpioAddress *address, const GpioMode pin_mode, GpioState init_state) {
  return STATUS_CODE_OK;
}

void setup_test(void) {
  memset(&s_test_storage, 0, sizeof(s_test_storage));
  memset(&s_test_config, 0, sizeof(s_test_config));
  g_motor_enable_state = GPIO_STATE_LOW;
  g_solar_enable_state = GPIO_STATE_LOW;
  g_pos_enable_state = GPIO_STATE_LOW;
  g_neg_enable_state = GPIO_STATE_LOW;
  g_killswitch_state = GPIO_STATE_HIGH;  // Not active

  // Initialize all modules needed for tests
  rear_controller_init(&s_test_storage, &s_test_config);
  rear_controller_state_manager_init();
}

void teardown_test(void) {
  rear_controller_deinit();
}

// --- Rear Controller and Relay Tests ---

TEST_IN_TASK
void test_rear_controller_init_sets_default_states(void) {
  TEST_ASSERT_FALSE(s_test_storage.pos_relay_closed);
  TEST_ASSERT_FALSE(s_test_storage.neg_relay_closed);
  TEST_ASSERT_FALSE(s_test_storage.solar_relay_closed);
  TEST_ASSERT_FALSE(s_test_storage.motor_relay_closed);
}

TEST_IN_TASK
void test_rear_fault_opens_all_relays(void) {
  // Close relays
  rear_pos_close();
  rear_neg_close();
  rear_motor_close();
  rear_solar_close();

  // Trigger fault
  rear_fault();

  // Assert all are now open
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, g_pos_enable_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, g_neg_enable_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, g_motor_enable_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, g_solar_enable_state);
}

TEST_IN_TASK
void test_rear_controller_pos_close_and_open(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, rear_pos_close());
  TEST_ASSERT_TRUE(s_test_storage.pos_relay_closed);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, rear_pos_open());
  TEST_ASSERT_FALSE(s_test_storage.pos_relay_closed);
}

TEST_IN_TASK
void test_rear_controller_solar_close_and_open(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, rear_solar_close());
  TEST_ASSERT_TRUE(s_test_storage.solar_relay_closed);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, rear_solar_open());
  TEST_ASSERT_FALSE(s_test_storage.solar_relay_closed);
}

TEST_IN_TASK
void test_rear_controller_motor_close_and_open(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, rear_motor_close());
  TEST_ASSERT_TRUE(s_test_storage.motor_relay_closed);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, rear_motor_open());
  TEST_ASSERT_FALSE(s_test_storage.motor_relay_closed);
}

TEST_IN_TASK
void test_relays_is_killswitch_active(void) {
  g_killswitch_state = GPIO_STATE_HIGH;  // Not active
  TEST_ASSERT_FALSE(relays_is_killswitch_active());

  g_killswitch_state = GPIO_STATE_LOW;  // Active
  TEST_ASSERT_TRUE(relays_is_killswitch_active());
}

// --- State Machine Tests ---

TEST_IN_TASK
void test_rear_sm_init_state(void) {
  TEST_ASSERT_EQUAL(REAR_CONTROLLER_STATE_INIT, rear_controller_state_manager_get_state());
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, g_pos_enable_state);  // Check that fault was called
}

TEST_IN_TASK
void test_rear_sm_init_to_precharge(void) {
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_INIT_COMPLETE);
  TEST_ASSERT_EQUAL(REAR_CONTROLLER_STATE_PRECHARGE, rear_controller_state_manager_get_state());
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, g_pos_enable_state);
}

TEST_IN_TASK
void test_rear_sm_precharge_to_idle(void) {
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_INIT_COMPLETE);

  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_PRECHARGE_SUCCESS);
  TEST_ASSERT_EQUAL(REAR_CONTROLLER_STATE_IDLE, rear_controller_state_manager_get_state());
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, g_neg_enable_state);
}

TEST_IN_TASK
void test_rear_sm_idle_to_drive(void) {
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_INIT_COMPLETE);
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_PRECHARGE_SUCCESS);

  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_DRIVE_REQUEST);
  TEST_ASSERT_EQUAL(REAR_CONTROLLER_STATE_DRIVE, rear_controller_state_manager_get_state());
  // Verify entry actions for DRIVE
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, g_motor_enable_state);
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, g_solar_enable_state);
}
