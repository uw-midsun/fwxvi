#include "light_signal_manager.h"

static LightsSignalState current_state = LIGHTS_SIGNAL_STATE_OFF;
static LightsSignalRequest current_request = LIGHTS_SIGNAL_REQUEST_NONE;

typedef enum {
    LIGHTS_SIGNAL_STATE_OFF = 0,
    LIGHTS_SIGNAL_STATE_LEFT,
    LIGHTS_SIGNAL_STATE_RIGHT,
    LIGHTS_SIGNAL_STATE_HAZARD
} LightsSignalState;

typedef enum {
    LIGHTS_SIGNAL_REQUEST_NONE = 0,
    LIGHTS_SIGNAL_REQUEST_LEFT,
    LIGHTS_SIGNAL_REQUEST_RIGHT,
    LIGHTS_SIGNAL_REQUEST_HAZARD
} LightsSignalRequest;

void lights_signal_manager_init(void) {
    current_state = LIGHTS_SIGNAL_STATE_OFF;
    current_request = LIGHTS_SIGNAL_REQUEST_NONE;    
}

void lights_signal_manager_request(LightsSignalRequest req) {
    if (is_valid_request(req)) {
        current_request = req;
    }
}

static bool is_valid_request(LightsSignalRequest req) {
    if (current_state == LIGHTS_SIGNAL_STATE_HAZARD &&
        (req == LIGHTS_SIGNAL_REQUEST_LEFT || req == LIGHTS_SIGNAL_REQUEST_RIGHT)) {
        return false;
    }
    switch (req) {
        case LIGHTS_SIGNAL_REQUEST_NONE:
            return true;
        case LIGHTS_SIGNAL_REQUEST_LEFT:
            return current_state != LIGHTS_SIGNAL_STATE_HAZARD;
        case LIGHTS_SIGNAL_REQUEST_RIGHT:
            return current_state != LIGHTS_SIGNAL_STATE_HAZARD;
        case LIGHTS_SIGNAL_REQUEST_HAZARD:
            return true;
        default:
            return false;        
    }
}

void lights_signal_manager_update(void) {
    switch (current_request) {
        case LIGHTS_SIGNAL_REQUEST_NONE:
            break;
        case LIGHTS_SIGNAL_REQUEST_LEFT:
            if (current_state == LIGHTS_SIGNAL_STATE_LEFT) {
                current_state = LIGHTS_SIGNAL_STATE_OFF;
            }
            else if (current_state == LIGHTS_SIGNAL_REQUEST_HAZARD) {
                
            }
            else {
                current_state = LIGHTS_SIGNAL_STATE_LEFT;
            }
            break;
        case LIGHTS_SIGNAL_REQUEST_RIGHT:
            if (current_state == LIGHTS_SIGNAL_STATE_RIGHT) {
                current_state = LIGHTS_SIGNAL_STATE_OFF;
            }
            else if (current_state == LIGHTS_SIGNAL_REQUEST_HAZARD) {
                
            }
            else {
                current_state = LIGHTS_SIGNAL_STATE_RIGHT;
            }
            break;
        case LIGHTS_SIGNAL_REQUEST_HAZARD:
            if (current_state == LIGHTS_SIGNAL_STATE_HAZARD) {
                current_state = LIGHTS_SIGNAL_STATE_OFF;
            }
            else {
                current_state = LIGHTS_SIGNAL_STATE_HAZARD;
            }
            break;
    }
}

LightsSignalState lights_signal_manager_get_state(void) {
    return current_state;
}