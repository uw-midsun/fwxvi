#include "fsm.h"


#include <limits.h>
#include <stdio.h>


#include "log.h"
#include "notify.h"


StatusCode fsm_transition(Fsm *fsm, StateId from, StateId to) {
  if (!fsm || to >= fsm->num_states || from >= fsm->num_states) return STATUS_CODE_INVALID_ARGS;
  if (xSemaphoreTake(fsm->fsm_sem, FSM_TIMEOUT_MS) != pdTRUE) return STATUS_CODE_TIMEOUT;
  bool transition_exists = false;
  for (uint8_t i = 0; i < MAX_TRANSITIONS; i++) {
    if (fsm->transition_table[fsm->curr_state * MAX_STATES + i] == to) {
      transition_exists = true;
      break;
    }
  }


  if (!transition_exists) {
    xSemaphoreGive(fsm->fsm_sem);
    return STATUS_CODE_INVALID_ARGS;
  }


  if (fsm->states[to].entry_func) {
    fsm->states[to].entry_func(fsm->context);
  }
  fsm->curr_state = to;


  xSemaphoreGive(fsm->fsm_sem);
  return STATUS_CODE_OK;
}


void fsm_run_cycle(Fsm *fsm) {
  if (fsm == NULL || fsm->fsm_sem == NULL) {
    LOG_CRITICAL("not configured, cannot run cycle\n");
    return;
  }


  Fsm *task_fsm = fsm->context;
  BaseType_t ret = xSemaphoreGive(fsm->fsm_sem);
  if (ret == pdFALSE) {
    LOG_CRITICAL("cycle trigger failed\n");
  }
}


void _fsm_task(void *context) {
  Fsm *self = (Fsm *)context;
  if (!self) LOG_DEBUG("Invalid context pointer\n");


  BaseType_t ret;


  while (true) {
    ret = xSemaphoreTake(self->fsm_sem, pdMS_TO_TICKS(FSM_TIMEOUT_MS));
    if (ret == pdTRUE) {
      if (self->states[self->curr_state].state_action != NULL) {
        self->states[self->curr_state].state_action(self->context);
      }
    } else {
      LOG_DEBUG("FSM timeout: No state transition occurred\n");
    }
    send_task_end();
  }
}


StatusCode fsm_init(Fsm *fsm, State *states, uint8_t *transitions, StateId initial_state, void *context) {
  if (fsm == NULL || states == NULL || transitions == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }


  if (initial_state >= MAX_STATES) {
    return STATUS_CODE_INVALID_ARGS;
  }


  fsm->context = context;
  fsm->states = states;
  fsm->transition_table = transitions;
  fsm->num_states = MAX_STATES;


  fsm->curr_state = initial_state;
  fsm->fsm_sem = xSemaphoreCreateCountingStatic(CYCLE_RX_MAX, 0, &fsm->sem_buf);
  return STATUS_CODE_OK;
}


StatusCode fsm_add_state(Fsm *fsm, StateId state_id, StateAction entry_func, StateAction state_func) {
  if (!fsm || state_id >= MAX_STATES || !entry_func || !state_func || state_id > 1) {
    return STATUS_CODE_INVALID_ARGS;
  }


  // check if states already exist


  if (fsm->states[state_id].state_action != NULL) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }


  fsm->states[state_id].id = state_id;
  fsm->states[state_id].entry_func = entry_func;
  fsm->states[state_id].state_action = state_func;


  const StateId target_state = (state_id == 0) ? 1 : 0;
  fsm->transition_table[state_id * MAX_TRANSITIONS] = target_state;


  // add ids to transition table


  for (uint8_t i = 0; i < MAX_TRANSITIONS; i++) {
    fsm->transition_table[state_id * MAX_TRANSITIONS + i] = 0xff;
  }


  fsm->num_states++;


  return STATUS_CODE_OK;
}


StatusCode fsm_add_transition(Fsm *fsm, StateId from, StateId to) {
  if (!fsm || from >= fsm->num_states || to >= fsm->num_states || to == from) {
    return STATUS_CODE_INVALID_ARGS;
  }


  // make sure states aleady exist
  if (fsm->states[from].state_action == NULL || fsm->states[to].state_action == NULL) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }


  fsm->transition_table[from * MAX_TRANSITIONS + to] = 1;


  return STATUS_CODE_OK;
}

