/************************************************************************************************
 * @file   test_fsm.c
 *
 * @brief  Test file for FSM
 *
 * @date   2025-02-06
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard Library Headers */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* Inter-component Headers */
#include "fsm.h"
#include "test_helpers.h"
#include "unity.h"

/* Test FSM setup */
static Fsm fsm;

typedef enum {
  TEST_STATE_0,
  TEST_STATE_1,
  TEST_STATE_2,
  TEST_STATE_3,
  TEST_STATE_4,
  TEST_STATE_5,
  TEST_STATE_6,
  TEST_STATE_7,
  TEST_STATE_8,
  TEST_STATE_9,
} TestStates;

/* Dummy state functions */
void test_state_fn(void *context) {
  (void)context;
}
void test_entry_fn(void *context) {
  (void)context;
}

/* Define FSM states correctly */
static State states[MAX_STATES] = { { TEST_STATE_0, test_entry_fn, test_state_fn }, { TEST_STATE_1, test_entry_fn, test_state_fn }, { TEST_STATE_2, test_entry_fn, test_state_fn },
                                    { TEST_STATE_3, test_entry_fn, test_state_fn }, { TEST_STATE_4, test_entry_fn, test_state_fn }, { TEST_STATE_5, test_entry_fn, test_state_fn },
                                    { TEST_STATE_6, test_entry_fn, test_state_fn }, { TEST_STATE_7, test_entry_fn, test_state_fn }, { TEST_STATE_8, test_entry_fn, test_state_fn },
                                    { TEST_STATE_9, test_entry_fn, test_state_fn } };

/* Transition Table */
static uint8_t transition_table[MAX_STATES * MAX_STATES] = {};

/* Test Context */
static int test_context = 0;

/* Setup & Teardown */
void setup_test(void) {
  fsm_init(&fsm, states, transition_table, TEST_STATE_0, NULL);
}
void teardown_test(void) {}

/* FSM Initialization Test */
TEST_IN_TASK
void test_fsm_initialization(void) {
  StatusCode status = fsm_init(&fsm, states, transition_table, TEST_STATE_0, NULL);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(TEST_STATE_0, fsm.curr_state);
  TEST_ASSERT_EQUAL_PTR(states, fsm.states);
  TEST_ASSERT_EQUAL_PTR(transition_table, fsm.transition_table);
}


TEST_IN_TASK
void test_fsm_add_transition(void){
  StatusCode status = fsm_add_transition(&fsm, TEST_STATE_0, TEST_STATE_1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(1, transition_table[TEST_STATE_0 * MAX_STATES + TEST_STATE_1]);
}

/* Invalid State Initialization */
TEST_IN_TASK
void test_fsm_initialize_with_invalid_state(void) {
  StatusCode status = fsm_init(&fsm, states, transition_table, MAX_STATES, NULL);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, status);
}

/* Valid FSM State Transition */
TEST_IN_TASK
void test_fsm_change_state_valid(void) {
  fsm_init(&fsm, states, transition_table, TEST_STATE_0, NULL);
  fsm_run_cycle(&fsm);
  StatusCode status = fsm_transition(&fsm, TEST_STATE_0, TEST_STATE_1);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL(TEST_STATE_1, fsm.curr_state);
}

/* Invalid State Transition */
TEST_IN_TASK
void test_fsm_transition_invalid(void) {
  fsm_init(&fsm, states, transition_table, TEST_STATE_0, NULL);
  fsm_run_cycle(&fsm);
  StatusCode status = fsm_transition(&fsm, TEST_STATE_0, MAX_STATES + 1);  // Invalid state
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, status);
}

/* Transition to an unreachable state */
TEST_IN_TASK
void test_fsm_transition_unreachable_state(void) {
  fsm_init(&fsm, states, transition_table, TEST_STATE_0, NULL);
  fsm_run_cycle(&fsm);
  StatusCode status = fsm_transition(&fsm, TEST_STATE_0, TEST_STATE_5);  // Unreachable state
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, status);
}