/************************************************************************************************
 * @file   fsm.c
 *
 * @brief  Implementation of the Finite State Machine (FSM) library
 *
 * @date   2024-10-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard Library Headers */
#include "fsm.h"
#include <string.h>

/*
 * @brief  Initializes the FSM with given states, transition table, and initial state.
 * @param  fsm: Pointer to FSM instance.
 * @param  states: Array of state structures.
 * @param  transitions: Transition table defining valid state transitions.
 * @param  initial_state: The starting state for the FSM.
 * @param  context: User-defined context pointer.
 * @return STATUS_CODE_OK on success, STATUS_CODE_INVALID_ARGS if invalid parameters are provided.
 */
StatusCode fsm_init(Fsm *fsm, State *states, uint8_t *transitions, StateId initial_state, void *context) {
    if (!fsm || !states || !transitions || initial_state >= MAX_STATES) {
        return STATUS_CODE_INVALID_ARGS;
    }
    
    memset(fsm, 0, sizeof(Fsm));
    fsm->states = states;
    fsm->transition_table = transitions;
    fsm->curr_state = initial_state;
    fsm->context = context;
    fsm->fsm_sem = xSemaphoreCreateMutexStatic(&fsm->sem_buf);
    
    return STATUS_CODE_OK;
}

/*
 * @brief  Adds a new state to the FSM.
 * @param  fsm: Pointer to FSM instance.
 * @param  state_id: ID of the new state.
 * @param  entry_func: Function executed upon entering the state.
 * @param  state_func: Function executed while in the state.
 * @return STATUS_CODE_OK on success, STATUS_CODE_INVALID_ARGS if parameters are invalid.
 */
StatusCode fsm_add_state(Fsm *fsm, StateId state_id, StateAction entry_func, StateAction state_func) {
    if (!fsm || state_id >= MAX_STATES) {
        return STATUS_CODE_INVALID_ARGS;
    }
    
    fsm->states[state_id].id = state_id;
    fsm->states[state_id].entry_func = entry_func;
    fsm->states[state_id].state_action = state_func;
    
    return STATUS_CODE_OK;
}

/*
 * @brief  Adds a valid transition between two states in the FSM.
 * @param  fsm: Pointer to FSM instance.
 * @param  from: The starting state.
 * @param  to: The target state.
 * @return STATUS_CODE_OK on success, STATUS_CODE_INVALID_ARGS if parameters are invalid.
 */
StatusCode fsm_add_transition(Fsm *fsm, StateId from, StateId to) {
    if (!fsm || from >= MAX_STATES || to >= MAX_STATES) {
        return STATUS_CODE_INVALID_ARGS;
    }
    
    fsm->transition_table[from] |= (1 << to);
    return STATUS_CODE_OK;
}

/*
 * @brief  Transitions FSM from the current state to a new state if valid.
 * @param  fsm: Pointer to FSM instance.
 * @param  to: Target state to transition to.
 * @return STATUS_CODE_OK if transition succeeds, STATUS_CODE_UNREACHABLE if transition is invalid.
 */
StatusCode fsm_transition(Fsm *fsm, StateId to) {
    if (!fsm || to >= MAX_STATES) {
        return STATUS_CODE_INVALID_ARGS;
    }
    
    if (!(fsm->transition_table[fsm->curr_state] & (1 << to))) {
        return STATUS_CODE_UNREACHABLE;
    }
    
    fsm->curr_state = to;
    return STATUS_CODE_OK;
}

/*
 * @brief  Executes the FSM cycle by running the current state's action function.
 * @param  fsm: Pointer to FSM instance.
 */
void fsm_run_cycle(Fsm *fsm) {
    if (!fsm) return;
    
    if (fsm->states[fsm->curr_state].state_action) {
        fsm->states[fsm->curr_state].state_action(fsm->context);
    }
}

/*
 * @brief  Internal FSM task function for FreeRTOS integration.
 * @param  context: Pointer to FSM instance.
 */
void _fsm_task(void *context) {
    Fsm *fsm = (Fsm *)context;
    if (!fsm) return;
    
    while (1) {
        if (xSemaphoreTake(fsm->fsm_sem, FSM_TIMEOUT_MS) == pdTRUE) {
            fsm_run_cycle(fsm);
            xSemaphoreGive(fsm->fsm_sem);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
