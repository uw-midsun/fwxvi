/************************************************************************************************
 * @file   test_drive_state_manager.c
 *
 * @brief  Test file for drive state manager
 *
 * @date   2025-07-22
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "unity.h"
#include "test_helpers.h"

/* Intra-component Headers */
#include "drive_manager.h"

void setup_test(void) {
    drive_state_manager_init();
}

void teardown_test(void) {}


TEST_IN_TASK
void test_neutral_to_drive(void) {
    drive_state_manager_request(DRIVE_STATE_REQUEST_D);
    drive_state_manager_update();
    TEST_ASSERT_EQUAL(DRIVE_STATE_DRIVE, drive_state_manager_get_state());
}

TEST_IN_TASK
void test_neutral_to_reverse(void) {
    drive_state_manager_request(DRIVE_STATE_REQUEST_R);
    drive_state_manager_update();
    TEST_ASSERT_EQUAL(DRIVE_STATE_REVERSE, drive_state_manager_get_state());
}

TEST_IN_TASK
void test_drive_to_reverse(void) {
    drive_state_manager_request(DRIVE_STATE_REQUEST_D);
    drive_state_manager_update();
    drive_state_manager_request(DRIVE_STATE_REQUEST_R);
    drive_state_manager_update();
    TEST_ASSERT_EQUAL(DRIVE_STATE_REVERSE, drive_state_manager_get_state());
}

TEST_IN_TASK
void test_drive_to_neutral(void) {
    drive_state_manager_request(DRIVE_STATE_REQUEST_D);
    drive_state_manager_update();
    drive_state_manager_request(DRIVE_STATE_REQUEST_N);
    drive_state_manager_update();
    TEST_ASSERT_EQUAL(DRIVE_STATE_NEUTRAL, drive_state_manager_get_state());
}

TEST_IN_TASK
void test_reverse_to_drive(void) {
    drive_state_manager_request(DRIVE_STATE_REQUEST_R);
    drive_state_manager_update();
    drive_state_manager_request(DRIVE_STATE_REQUEST_D);
    drive_state_manager_update();
    TEST_ASSERT_EQUAL(DRIVE_STATE_DRIVE, drive_state_manager_get_state());
}

TEST_IN_TASK
void test_reverse_to_neutral(void) {
    drive_state_manager_request(DRIVE_STATE_REQUEST_R);
    drive_state_manager_update();
    drive_state_manager_request(DRIVE_STATE_REQUEST_N);
    drive_state_manager_update();
    TEST_ASSERT_EQUAL(DRIVE_STATE_NEUTRAL, drive_state_manager_get_state());
}