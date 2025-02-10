#pragma once

/************************************************************************************************
 * @file   fsm.h
 *
 * @brief  Finite State Machine Library
 *
 * @date   2024-10-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include "stdint.h"

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "semphr.h"

/* Intra-component Headers */
#include "status.h"
#include "tasks.h"

/**
 * @defgroup Finite_State_Machine
 * @brief    FSM library
 * @{
 */

#define MAX_STATES 10
#define MAX_TRANSITIONS 5
#define FSM_TIMEOUT_MS 1000
#define CYCLE_RX_MAX 15

typedef void (*StateAction)(void *context);
typedef uint8_t StateId;

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

/**
 * @brief   Creates a finite state machine (FSM) with a specified number of states.
 * @details This macro defines an FSM instance and creates a corresponding task.
 * 
 * @param   name         Name of the FSM instance.
 * @param   num_fsm_states Number of states in the FSM.
 * @param   stack_size   Stack size for the FSM task.
 */
#define FSM(name, num_fsm_states, stack_size) \
  Fsm *name##_fsm = &((Fsm){                  \
      .num_states = num_fsm_states,           \
  });                                         \
  TASK(name, stack_size) {                    \
    _fsm_task(context);                       \
  }

/**
 * @brief   Defines a transition between two states.
 * @details The transition must exist in the transition table.
 *
 * @param   from   State ID representing the starting state.
 * @param   to     State ID representing the destination state.
 */
#define TRANSITION(from, to);

/**
 * @brief   Defines a state with an associated ID, inputs, and outputs.
 * @details The state ID must be unique (preferably an enum value).
 *
 * @param   StateId   Unique ID representing the state.
 * @param   inputs    Function executed upon entering the state.
 * @param   outputs   Function executed while the state is active.
 */
#define STATE(StateId, inputs, outputs) \
  { .id = STATE_ID, .inputs = entry_func, .outputs = state_action }

/**
 * @brief   Initializes a finite state machine.
 * @details Sets up the FSM with states, transitions, an initial state, and an execution context.
 *
 * @param   fsm            Pointer to the FSM structure.
 * @param   states         Array of states in the FSM.
 * @param   transitions    Transition table defining valid state transitions.
 * @param   initial_state  Initial state ID of the FSM.
 * @param   context        Context pointer passed to state functions.
 *
 * @return  StatusCode indicating success or failure.
 */
StatusCode fsm_init(Fsm *fsm, State *states, uint8_t *transitions, StateId initial_state, void *context);

/**
 * @brief   Performs a state transition in the FSM.
 * @details Changes the FSM state if the transition is valid.
 *
 * @param   fsm   Pointer to the FSM structure.
 * @param   from  Current state ID.
 * @param   to    Target state ID.
 *
 * @return  StatusCode indicating success or failure.
 */
StatusCode fsm_transition(Fsm *fsm, StateId from, StateId to);

/**
 * @brief   Executes one cycle of the FSM.
 * @details Calls the current state's action function and handles state transitions.
 *
 * @param   fsm   Pointer to the FSM structure.
 */
void fsm_run_cycle(Fsm *fsm);

/**
 * @brief   FSM task function executed in the task loop.
 * @details Manages the FSM state transitions and executes state logic.
 *
 * @param   context   Pointer to the FSM context.
 */
void _fsm_task(void *context);

/**
 * @brief   Adds a new state to the FSM.
 * @details Registers a state with its entry and action functions.
 *
 * @param   fsm         Pointer to the FSM structure.
 * @param   state_id    Unique ID representing the state.
 * @param   entry_func  Function executed when entering the state.
 * @param   state_func  Function executed while the state is active.
 *
 * @return  StatusCode indicating success or failure.
 */
StatusCode fsm_add_state(Fsm *fsm, StateId state_id, StateAction entry_func, StateAction state_func);

/**
 * @brief   Adds a transition between two states in the FSM.
 * @details The transition must be valid in the transition table.
 *
 * @param   fsm   Pointer to the FSM structure.
 * @param   from  State ID representing the starting state.
 * @param   to    State ID representing the destination state.
 *
 * @return  StatusCode indicating success or failure.
 */
StatusCode fsm_add_transition(Fsm *fsm, StateId from, StateId to);
