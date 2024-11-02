#pragma once

/************************************************************************************************
 * fsm.h
 *
 * Finite State Machine Library
 *
 * Created: 2024-10-27
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include "stdint.h"

/* Inter-component Headers */
#include "semphr.h"

/* Intra-component Headers */
#include "tasks.h"
#include "status.h"

#define MAX_STATES 10
#define MAX_TRANSITIONS 5

typedef void (*StateAction)(void* context);
typedef uint8_t StateId;

typedef struct {

} Transition;

typedef struct {
    StateId id;
    StateAction entry_func;
    StateAction state_action;
} State;

typedef struct {
    State *states;
    uint8_t *transition_table;
    void *context;
    StateId curr_state;
    SemaphoreHandle_t fsm_sem;
    StaticSemaphore_t sem_buf;
    uint8_t num_states;
} Fsm;

StatusCode fsm_init(Fsm *fsm, void *context);
StatusCode fsm_add_state(Fsm *fsm);
StatusCode fsm_add_transition();
StatusCode fsm_start(Fsm *fsm, StateId initial_state);
void fsm_run_cycle(Fsm *fsm);

/**
 * @brief   Creates state with associated ID in a state list
 * @details State id must be unique (preferred to use enum type)
 */
#define STATE(state_id, entry, state_func) \
  [state_id] = { .entry_func = entry, .state_action = state_func }