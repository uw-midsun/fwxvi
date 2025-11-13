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

    if (steering_storage->cruise_control_enabled) {
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

    if (steering_storage->cruise_control_enabled) {
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
        LOG_DEBUG("Cruise control enabled\r\n");

        if (steering_storage->cruise_control_enabled) {
            steering_storage->cruise_control_enabled = false;
        } else {
            steering_storage->cruise_control_enabled = true;
        }
    }

    return STATUS_CODE_OK;
}
