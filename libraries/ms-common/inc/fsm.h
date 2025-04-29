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
 * @brief   Creates state with associated ID in a state list
 * @details State id must be unique (preferred to use enum type)
 */
#define STATE(state_id, entry, state_func) [state_id] = { .entry_func = entry, .state_action = state_func }

/** @} */
