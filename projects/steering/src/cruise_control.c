/************************************************************************************************
 * @file    cruise_control.c
 *
 * @brief   Cruise Control
 *
 * @date    2025-11-12
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "log.h"

/* Intra-component Headers */
#include "button.h"
#include "steering.h"
#include "button_manager.h"

static SteeringStorage *steering_storage;
bool up_prev_state = false;
bool down_prev_state = false;
int counter = 0;

StatusCode cruise_control_init(SteeringStorage *storage) {
    if (storage == NULL) {
        return STATUS_CODE_INVALID_ARGS;
    }

    steering_storage = storage;
    return STATUS_CODE_OK;
}

StatusCode cruise_control_down_handler() {
    if (steering_storage == NULL) {
        return STATUS_CODE_UNINITIALIZED;
    }

    if (steering_storage->cruise_control_enabled && steering_storage->cruise_control_target_speed_kmh > steering_storage->config->cruise_min_speed_kmh) {
        steering_storage->cruise_control_target_speed_kmh--;
    } else {
        return STATUS_CODE_RESOURCE_EXHAUSTED;
    }

    return STATUS_CODE_OK;
};

StatusCode cruise_control_up_handler() {
    if (steering_storage == NULL) {
        return STATUS_CODE_UNINITIALIZED;
    }

    if (steering_storage->cruise_control_enabled && steering_storage->cruise_control_target_speed_kmh < steering_storage->config->cruise_max_speed_kmh) {
        steering_storage->cruise_control_target_speed_kmh++;
    } else {
        return STATUS_CODE_RESOURCE_EXHAUSTED;
    }

    return STATUS_CODE_OK;
}

StatusCode cruise_control_run() {

    if (steering_storage == NULL) {
        return STATUS_CODE_UNINITIALIZED;
    }

    LOG_DEBUG("Cruise control target speed: %u\r\n", steering_storage->cruise_control_target_speed_kmh);

    if(steering_storage->button_manager->buttons[STEERING_BUTTON_CRUISE_CONTROL_UP].state == BUTTON_PRESSED && steering_storage->button_manager->buttons[STEERING_BUTTON_CRUISE_CONTROL_DOWN].state == BUTTON_PRESSED){
        
        if (steering_storage->cruise_control_enabled) {
            LOG_DEBUG("Cruise control disabled\r\n");
            steering_storage->cruise_control_enabled = false;
        } else {
            LOG_DEBUG("Cruise control enabled\r\n");
            steering_storage->cruise_control_enabled = true;
        }
    
    
    } else if (steering_storage->cruise_control_enabled) {
        
        
        if(steering_storage->button_manager->buttons[STEERING_BUTTON_CRUISE_CONTROL_UP].state == BUTTON_PRESSED){
            if(up_prev_state){
                counter++;
                for(int i = 0; i < counter; i++){
                    cruise_control_up_handler();
                }
            } else {
                counter = 0;
                cruise_control_up_handler();
            }
        } 

        else if(steering_storage->button_manager->buttons[STEERING_BUTTON_CRUISE_CONTROL_DOWN].state == BUTTON_PRESSED){
            if(down_prev_state){
                counter++;
                for(int i = 0; i < counter; i++){
                    cruise_control_down_handler();
                }
            } else {
                counter = 0;
                cruise_control_down_handler();
            }
        }

        up_prev_state = (steering_storage->button_manager->buttons[STEERING_BUTTON_CRUISE_CONTROL_UP].state == BUTTON_PRESSED);
        down_prev_state = (steering_storage->button_manager->buttons[STEERING_BUTTON_CRUISE_CONTROL_DOWN].state == BUTTON_PRESSED);

        /*
            if (steering_storage->button_manager->buttons[STEERING_BUTTON_CRUISE_CONTROL_UP].state == BUTTON_PRESSED) {
            cruise_control_up_handler();
        } else if (steering_storage->button_manager->buttons[STEERING_BUTTON_CRUISE_CONTROL_DOWN].state == BUTTON_PRESSED) {
            cruise_control_down_handler();
        }
        */

    }
    

    return STATUS_CODE_OK;
}
