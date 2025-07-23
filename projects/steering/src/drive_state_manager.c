/************************************************************************************************
 * @file   drive_state_manager.c
 *
 * @brief  Main file for drive state manager
 *
 * @date   2025-07-22
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

#include <drive_manager.h>
#include <stdbool.h>


static DriveState current_state = DRIVE_STATE_NEUTRAL;
static DriveStateRequest current_request = DRIVE_STATE_REQUEST_NONE;

void drive_state_manager_init(void) {
      current_state = DRIVE_STATE_NEUTRAL;
      current_request = DRIVE_STATE_REQUEST_NONE;
}

void drive_state_manager_request(DriveStateRequest req) {
    current_request = req;
}

void drive_state_manager_update(void) {
    switch (current_request) {
    case DRIVE_STATE_REQUEST_D:
      current_state = DRIVE_STATE_DRIVE;
      break;
    case DRIVE_STATE_REQUEST_N:
      current_state = DRIVE_STATE_NEUTRAL;
      break;
    case DRIVE_STATE_REQUEST_R:
      current_state = DRIVE_STATE_REVERSE;
      break;
    default:
      current_state = DRIVE_STATE_INVALID;
      break;
  }
}

DriveState drive_state_manager_get_state(void) {
    return current_state;
}
