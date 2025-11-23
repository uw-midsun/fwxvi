#include "motor_can.h"

#include "ws22_motor_can.h"
#include "front_controller.h"

static FrontControllerStorage *frontControllerStorage; 

static void prv_update_target_current_velocity() {

    DriveState currentState = frontControllerStorage->currentDriveState;

    switch(currentState){
        case DRIVE_STATE_DRIVE:
            ws22_motor_can_set_current(frontControllerStorage->accel_percentage);
            ws22_motor_can_set_velocity(WS22_CONTROLLER_MAX_VELOCITY);
            break;
        case DRIVE_STATE_REVERSE:
            ws22_motor_can_set_current(frontControllerStorage->accel_percentage);
            ws22_motor_can_set_velocity(-WS22_CONTROLLER_MAX_VELOCITY);
            break;
        case DRIVE_STATE_CRUISE:
            ws22_motor_can_set_current(1.0f);
            ws22_motor_can_set_velocity(frontControllerStorage->cc_target_velocity_kph * VEL_TO_RPM_RATIO);
            break;
        case DRIVE_STATE_BRAKE:
            ws22_motor_can_set_current(0.0f);
            ws22_motor_can_set_velocity(0.0f);
            break;
        case DRIVE_STATE_REGEN:
            ws22_motor_can_set_current(frontControllerStorage->accel_percentage);
            ws22_motor_can_set_velocity(0.0f);
            break;
        case DRIVE_STATE_NEUTRAL:
            ws22_motor_can_set_current(0.0f);
            ws22_motor_can_set_velocity(0.0f);
            break;
        default:
            //invalid drive state
            return;
    }

}

StatusCode motor_can_init(FrontControllerStorage *storage){
    if (storage == NULL){
        return STATUS_CODE_INVALID_ARGS;
    }

    frontControllerStorage = storage;

    return STATUS_CODE_OK;
}