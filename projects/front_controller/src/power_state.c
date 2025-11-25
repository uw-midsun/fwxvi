#include "power_state.h"

#include "power_control_manager.h"
#include "front_controller_getters.h"

static FrontControllerStorage *frontControllerStorage;
static PowerState previous_power_state = POWER_STATE_INVALID;


static void set_load_switches(){
    uint8_t bps_fault_from_rear = get_rear_controller_status_bps_fault();
    uint8_t drive_state_from_steering = get_steering_buttons_drive_state();
    uint8_t is_precharge_complete_from_rear = get_battery_stats_B_motor_precharge_complete();

    if (bps_fault_from_rear == 1){
        frontControllerStorage->current_power_state = POWER_STATE_FAULT;
    }else if (drive_state_from_steering == DRIVE_STATE_DRIVE || drive_state_from_steering == DRIVE_STATE_CRUISE || drive_state_from_steering == DRIVE_STATE_REVERSE){
        if (is_precharge_complete_from_rear == 1){
            frontControllerStorage->current_power_state = POWER_STATE_ENGAGED;
        }else{
            frontControllerStorage->current_power_state = previous_power_state;
        }
    }else if(drive_state_from_steering == DRIVE_STATE_NEUTRAL){
        frontControllerStorage->current_power_state = POWER_STATE_IDLE;
    }

    if(frontControllerStorage->current_power_state == POWER_STATE_FAULT && previous_power_state != POWER_STATE_FAULT){
        power_control_set_output_group(OUTPUT_GROUP_ALL, false);
        power_control_set_output_group(HAZARD_GROUP, true);
    }else if(frontControllerStorage->current_power_state == POWER_STATE_ENGAGED && previous_power_state != POWER_STATE_ENGAGED){
        power_control_set_output_group(OUTPUT_GROUP_ALL, true);
    }else if(frontControllerStorage->current_power_state == POWER_STATE_IDLE && previous_power_state != POWER_STATE_IDLE){
        power_control_set_output_group(OUTPUT_GROUP_ALL, false);
        power_control_set_output_group(IDLE_GROUP, true);
    }

}


StatusCode power_state_init(FrontControllerStorage *storage){
    if (storage == NULL){
        return STATUS_CODE_INVALID_ARGS;
    }

    frontControllerStorage = storage;

    return STATUS_CODE_OK;
}