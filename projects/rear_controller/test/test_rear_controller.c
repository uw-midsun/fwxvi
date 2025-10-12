/************************************************************************************************
 * @file   test_rear_controller.c
 *
 * @brief  Test file for rear_controller
 *
 * @date   2025-05-28
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* Inter-component Headers */
#include "test_helpers.h"
#include "unity.h"
#include "log.h"
#include "delay.h"

/* Intra-component Headers */
#include "rear_controller.h"
#include "rear_controller_state_manager.h"
#include "rear_controller_hw_defs.h"
#include "relays.h"

static RearControllerStorage s_test_storage;
static RearControllerConfig s_test_config;


static GpioState g_motor_enable_state = GPIO_STATE_LOW;
static GpioState g_solar_enable_state = GPIO_STATE_LOW;
static GpioState g_pos_enable_state   = GPIO_STATE_LOW;
static GpioState g_neg_enable_state   = GPIO_STATE_LOW;
static GpioState g_killswitch_state   = GPIO_STATE_HIGH; // Default to not active

// Mock for delay_ms
void delay_ms(uint32_t ms) {
    // No-op for testing
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

StatusCode gpio_init_pin(const GpioAddress *address, const GpioMode pin_mode, GpioState init_state){
    return STATUS_CODE_OK;
}

void setup_test(void) {
    memset(&s_test_storage, 0, sizeof(s_test_storage));
    memset(&s_test_config, 0, sizeof(s_test_config));
    g_motor_enable_state = GPIO_STATE_LOW;
    g_solar_enable_state = GPIO_STATE_LOW;
    g_pos_enable_state   = GPIO_STATE_LOW;
    g_neg_enable_state   = GPIO_STATE_LOW;
    g_killswitch_state   = GPIO_STATE_HIGH; // Not active

    rear_controller_init(&s_test_storage, &s_test_config);
}

void teardown_test(void) {
    rear_controller_deinit();
}

TEST_IN_TASK
void test_rear_controller_init_sets_default_states(void){
    TEST_ASSERT_FALSE(s_test_storage.pos_relay_closed);
    TEST_ASSERT_FALSE(s_test_storage.neg_relay_closed);
    TEST_ASSERT_FALSE(s_test_storage.solar_relay_closed);
    TEST_ASSERT_FALSE(s_test_storage.motor_relay_closed);
}

TEST_IN_TASK
void test_rear_fault_opens_all_relays(void){

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, relays_close_motor());
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, relays_close_solar());
  s_test_storage.pos_relay_closed = true;
  s_test_storage.neg_relay_closed = true;

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, rear_fault());

  TEST_ASSERT_FALSE(s_test_storage.pos_relay_closed);
  TEST_ASSERT_FALSE(s_test_storage.neg_relay_closed);
  TEST_ASSERT_FALSE(s_test_storage.solar_relay_closed);
  TEST_ASSERT_FALSE(s_test_storage.motor_relay_closed);
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
    g_killswitch_state = GPIO_STATE_HIGH; // Not active
    TEST_ASSERT_FALSE(relays_is_killswitch_active());

    g_killswitch_state = GPIO_STATE_LOW; // Active
    TEST_ASSERT_TRUE(relays_is_killswitch_active());
}

TEST_IN_TASK
void test_relays_verify_states_ok_when_matching(void) {
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, relays_verify_states());

    TEST_ASSERT_EQUAL(STATUS_CODE_OK, relays_close_motor());
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, relays_verify_states());

    TEST_ASSERT_EQUAL(STATUS_CODE_OK, relays_close_solar());
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, relays_verify_states());
}

TEST_IN_TASK
void test_relays_verify_states_fails_on_mismatch(void) {
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, relays_close_motor()); 
    g_motor_enable_state = GPIO_STATE_LOW; 
    
    TEST_ASSERT_EQUAL(STATUS_CODE_INTERNAL_ERROR, relays_verify_states());
}