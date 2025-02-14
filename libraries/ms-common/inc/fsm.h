

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
#include
/* Intra-component Headers */
#include "status.h"
#include "tasks.h"


/**
 * @defgroup Finite_State_Machine
 * @brief    FSM library
 * @{
 */


#define MAX_STATES 10
 "stdint.h"


/* Inter-component Headers */
#include "FreeRTOS.h"
#include "semphr.h"
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


#define FSM(name, num_fsm_states, stack_size) \
  Fsm *name##_fsm = &((Fsm){                  \
      .num_states = num_fsm_states,           \
  });                                         \
  TASK(name, stack_size) {                    \
    _fsm_task(context);                       \
  }


  #define TRANSITION(from, to);
/**
 * @brief   Creates state with associated ID in a state list
 * @details State id must be unique (preferred to use enum type)
 */
#define STATE(StateId, inputs, outputs) \
  { .id = STATE_ID, .inputs = entry_func, .outputs = state_action }


StatusCode fsm_init(Fsm *fsm, State *states, uint8_t *transitions, StateId initial_state, void *context);


StatusCode fsm_transition(Fsm *fsm, StateId from, StateId to);


void fsm_run_cycle(Fsm *fsm);


void _fsm_task(void *context);


StatusCode fsm_add_state(Fsm *fsm, StateId state_id, StateAction entry_func, StateAction state_func);
StatusCode fsm_add_transition(Fsm *fsm, StateId from, StateId to);



