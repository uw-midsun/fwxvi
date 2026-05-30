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
#include "steering_getters.h"
#include "steering_setters.h"
#include "system_can.h"

/* Intra-component Headers */
#include "pedal_calib.h"
#include "steering.h"

/**
 * @brief Calibration state tracker
 */
typedef struct {
  bool active;
  bool subtitle_cleared;  // Set once rx() clears the "press to continue" subtitle
  uint8_t last_status;
} PedalCalibState;

static PedalCalibState s_calib_state = { false, false, 0xFF };

/**
 * @brief   Get user-facing status message text
 */
static const char *s_get_status_text(uint8_t status) {
  switch (status) {
    case 0:
      return "Initializing...";
    case 1:
      return "LIFT Accel\nPedal";
    case 2:
      return "PRESS Accel\nPedal";
    case 3:
      return "LIFT Accel\nPedal (2)";
    case 4:
      return "PRESS Accel\nPedal (2)";
    case 5:
      return "LIFT Brake\nPedal";
    case 6:
      return "PRESS Brake\nPedal";
    case 7:
      return "Calibration\nComplete!";
    case 8:
      return "Calibration\nFailed!";
    default:
      return "Unknown Status";
  }
}

StatusCode steering_pedal_calib_init(SteeringStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_calib_state.active = false;
  s_calib_state.subtitle_cleared = false;
  s_calib_state.last_status = 0xFF;

  return STATUS_CODE_OK;
}

StatusCode steering_pedal_calib_request(SteeringStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_calib_state.active) {
    return STATUS_CODE_OK;
  }

  s_calib_state.active = true;
  s_calib_state.subtitle_cleared = false;
  s_calib_state.last_status = 0xFF;

  // Write to CAN TX struct (safe to call from any task — single field write)
  set_pedal_calib_request_command(1);

  return STATUS_CODE_OK;
}

StatusCode steering_pedal_calib_rx(SteeringStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (!s_calib_state.active) {
    return STATUS_CODE_OK;
  }

  // Clear "press to continue" subtitle on first rx cycle after activation.
  // This is called from the display task so LVGL calls are safe here.
  if (!s_calib_state.subtitle_cleared) {
    gui_pedal_calib_widget_subtitle_text("");
    s_calib_state.subtitle_cleared = true;
  }

#ifdef MS_PLATFORM_X86
  // Debug: auto-advance through calibration states since there is no real CAN
  static uint32_t debug_call_counter = 0;
  uint8_t status = (s_calib_state.last_status == 0xFF) ? 0 : s_calib_state.last_status;
  debug_call_counter++;
  if (debug_call_counter > 60 && status < 7) {
    debug_call_counter = 0;
    status++;
  }
#else
  uint8_t status = (uint8_t)get_pedal_calib_status_status();
#endif

  if (status != s_calib_state.last_status) {
    s_calib_state.last_status = status;
    StatusCode update_status = gui_pedal_calib_widget_big_text(s_get_status_text(status));
    if (update_status != STATUS_CODE_OK) {
      LOG_DEBUG("Failed to update pedal calib display: %u\r\n", update_status);
    }

    if (status == 7 || status == 8) {
      set_pedal_calib_request_command(0);
      s_calib_state.active = false;
    }
  }

  return STATUS_CODE_OK;
}

bool steering_pedal_calib_is_active(SteeringStorage *storage) {
  (void)storage;
  return s_calib_state.active;
}
