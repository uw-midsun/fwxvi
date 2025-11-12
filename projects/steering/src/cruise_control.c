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

/* Intra-component Headers */
#include "button.h"
#include "steering.h"
#include "button_manager.c"

static SteeringStorage *steering_storage;

StatusCode cruise_control_init(SteeringStorage *storage) {
    if (storage == NULL) {
        return STATUS_CODE_INVALID_ARGS;
    }

    steering_storage = storage;
    return STATUS_CODE_OK;
}

StatusCode cruise_control_down_handler() {
    steering_storage -> cruise_control_target_speed_kmh = (steering_storage ->cruise_control_target_speed_kmh)--;
    return STATUS_CODE_OK;
};

StatusCode cruise_control_up_handler(){
    steering_storage -> cruise_control_target_speed_kmh = (steering_storage ->cruise_control_target_speed_kmh)++;
    return STATUS_CODE_OK;
}

StatusCode cruise_control_run(){
    if(steering_storage->button_manager->buttons[STEERING_BUTTON_CRUISE_CONTROL_UP].state == BUTTON_PRESSED && steering_storage->button_manager->buttons[STEERING_BUTTON_CRUISE_CONTROL_DOWN].state == BUTTON_PRESSED){
        steering_storage -> cruse_control_enabled == true;
    } else {
        steering_storage -> cruse_control_enabled == false;
    }
    return STATUS_CODE_OK;
}
