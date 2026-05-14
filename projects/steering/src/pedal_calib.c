/************************************************************************************************
 * @file   pedal_calib.c
 *
 * @brief  Steering-side pedal calibration handler implementation
 *
 * @date   2026-04-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <string.h>

/* Inter-component Headers */
#include "can.h"
#include "gui_pedal_calib_screen.h"
#include "gui_screens.h"
#include "log.h"
#include "status.h"
#include "system_can.h"

/* Intra-component Headers */
#include "pedal_calib.h"
#include "steering.h"

/**
 * @brief Calibration state tracker
 */
typedef struct {
  bool active;
  uint8_t last_status;
  uint32_t last_status_change_ms;  // For debug auto-advance
} PedalCalibState;

static PedalCalibState s_calib_state = { false, 0, 0 };

/**
 * @brief   Get status message text for display
 */
static const char *s_get_status_text(uint8_t status) {
  switch (status) {
    case 0:
      return "Initializing...";
    case 1:
      return "Lift Accel\nPedal UP";
    case 2:
      return "Push Accel\nPedal DOWN";
    case 3:
      return "Lift Accel\nPedal UP (2)";
    case 4:
      return "Push Accel\nPedal DOWN (2)";
    case 5:
      return "Lift Brake\nPedal UP";
    case 6:
      return "Push Brake\nPedal DOWN";
    case 7:
      return "Complete!\nCalibration Done";
    case 8:
      return "ERROR!\nCalibration Failed";
    default:
      return "Unknown Status";
  }
}

StatusCode steering_pedal_calib_init(SteeringStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_calib_state.active = false;
  s_calib_state.last_status = 0;

  return STATUS_CODE_OK;
}

StatusCode steering_pedal_calib_start(SteeringStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_calib_state.active = true;
  s_calib_state.last_status = 0;

  // Switch to pedal calibration screen
  StatusCode status = gui_screens_show(GUI_SCREEN_PEDAL_CALIB);
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Failed to switch to pedal calib screen: %u\r\n", status);
    return status;
  }

  // Update display with initial message
  status = gui_pedal_calib_widget_big_text("Initializing...");
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Failed to update pedal calib display: %u\r\n", status);
  }

  // Send CAN request to front controller to start calibration
  // The auto-generated function will be: set_steering_pedal_calib_request_command()
  // set_steering_pedal_calib_request_command(1);

  return STATUS_CODE_OK;
}

StatusCode steering_pedal_calib_rx(SteeringStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (!s_calib_state.active) {
    return STATUS_CODE_OK;
  }

  uint8_t old_status = s_calib_state.last_status;

  // Receive status from front controller
  // The auto-generated function will be: get_front_controller_pedal_calib_status_status()
  // uint8_t status = get_front_controller_pedal_calib_status_status();

#ifdef MS_PLATFORM_X86
  // Debug mode: Auto-advance through calibration states
  static uint32_t debug_call_counter = 0;
  
  debug_call_counter++;
  if (debug_call_counter > 60 && s_calib_state.last_status < 7) {
    debug_call_counter = 0;
    s_calib_state.last_status++;
  }
#endif

  uint8_t status = s_calib_state.last_status;

  // If status changed, update display
  if (status != old_status) {
    const char *status_text = s_get_status_text(status);
    StatusCode update_status = gui_pedal_calib_widget_big_text(status_text);
    if (update_status != STATUS_CODE_OK) {
      LOG_DEBUG("Failed to update pedal calib display: %u\r\n", update_status);
    }

    // Check if calibration is complete or errored
    if (status == 7 || status == 8) {
      s_calib_state.active = false;
    }
  }

  return STATUS_CODE_OK;
}

bool steering_pedal_calib_is_active(SteeringStorage *storage) {
  (void)storage;  // Not used
  return s_calib_state.active;
}
