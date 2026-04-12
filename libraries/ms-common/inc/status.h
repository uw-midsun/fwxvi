#pragma once

/************************************************************************************************
 * @file   status.h
 *
 * @brief  Status Library for more verbose error handling
 *
 * @date   2024-10-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup Status_Code
 * @brief    Global error library
 * @{
 */

/**
 * @brief StatusCodes for various errors.
 */
typedef enum {
  STATUS_CODE_OK = 0,
  STATUS_CODE_UNKNOWN,
  STATUS_CODE_INVALID_ARGS,
  STATUS_CODE_RESOURCE_EXHAUSTED,
  STATUS_CODE_UNREACHABLE,
  STATUS_CODE_TIMEOUT,
  STATUS_CODE_EMPTY,
  STATUS_CODE_OUT_OF_RANGE,
  STATUS_CODE_UNIMPLEMENTED,
  STATUS_CODE_UNINITIALIZED,
  STATUS_CODE_INTERNAL_ERROR,
  STATUS_CODE_INCOMPLETE,
  STATUS_CODE_ALREADY_INITIALIZED,
  NUM_STATUS_CODES,
} StatusCode;

typedef enum{

  FILE_ACCEL_PEDAL = 0,
  FILE_BRAKE_PEDAL,
  FILE_CRUISE_CONTROL,
  FILE_FONY_CONTROLLER_STATE_MANAGER,
  FILE_FRONT_CONTROLLER,
  FILE_FRONT_LIGHTS_SIGNAL,
  FILE_HORN,
  FILE_MAIN,
  FILE_MOTOR_CAN,
  FILE_OPD,
  FILE_PEDAL_CALIB_READER,
  FILE_POWER_MANAGER,
  FILE_WS22_MOTOR_CAN,
  FILE_BMI323,
  FILE_IMU,
  FILE_FAULT,
  FILE_CELL_SENSE,
  FILE_CURRENT_SENSE,
  FILE_FANS,
  FILE_KILLSWITCH,
  FILE_POWER_PATH_MANAGER,
  FILE_PRECHARGE,
  FILE_REAR_CONTROLLER_STATE_MANAGER,
  FILE_REAR_CONTROLLER,
  FILE_RELAYS,
  FILE_STATE_OF_CHANGE,
  FILE_THERMISTOR,
  FILE_BUTTON_LED,
  FILE_BUTTON_MANAGER,
  FILE_BUTTON,
  FILE_BUZZER,
  FILE_DISPLAY,
  FILE_DRIVE_STATE_MANAGER,
  FILE_LIGHT_SIGNAL_MANAGER,
  FILE_PARTY_MODE,
  FILE_RANGE_ESTIMATOR,
  FILE_STEERING,
  FILE_DATAGRAM,
  FILE_SD_CARD_INTERFACE,
  FILE_TELEMETRY,
  FILE_XB_TRANSMIT,
  FILE_NUM_FILES
  

} File;

typedef struct {
  char *name;
  File file;
} FileStringLut;

extern const FileStringLut *const file_string_lut[] = {

  {FILE_ACCEL_PEDAL, "accel_pedal.c"},
  {FILE_BRAKE_PEDAL, "brake_pedal.c"},
  {FILE_CRUISE_CONTROL, "cruise_control.c"},
  {FILE_FONY_CONTROLLER_STATE_MANAGER, "fony_controller_state_manager.c"},
  {FILE_FRONT_CONTROLLER, "front_controller.c"},
  {FILE_FRONT_LIGHTS_SIGNAL, "front_lights_signal.c"},
  {FILE_HORN, "horn.c"},
  {FILE_MAIN, "main.c"},
  {FILE_MOTOR_CAN, "motor_can.c"},
  {FILE_OPD, "opd.c"},
  {FILE_PEDAL_CALIB_READER, "pedal_calib_reader.c"},
  {FILE_POWER_MANAGER, "power_manager.c"},
  {FILE_WS22_MOTOR_CAN, "ws22_motor_can.c"},
  {FILE_BMI323, "bmi323.c"},
  {FILE_IMU, "imu.c"},
  {FILE_FAULT, "fault.c"},
  {FILE_CELL_SENSE, "cell_sense.c"},
  {FILE_CURRENT_SENSE, "current_sense.c"},
  {FILE_FANS, "fans.c"},
  {FILE_KILLSWITCH, "killswitch.c"},
  {FILE_POWER_PATH_MANAGER, "power_path_manager.c"},
  {FILE_PRECHARGE, "precharge.c"},
  {FILE_REAR_CONTROLLER_STATE_MANAGER, "rear_controller_state_manager.c"},
  {FILE_REAR_CONTROLLER, "rear_controller.c"},
  {FILE_RELAYS, "relays.c"},
  {FILE_STATE_OF_CHANGE, "state_of_change.c"},
  {FILE_THERMISTOR, "thermistor.c"},
  {FILE_BUTTON_LED, "button_led.c"},
  {FILE_BUTTON_MANAGER, "button_manager.c"},
  {FILE_BUTTON, "button.c"},
  {FILE_BUZZER, "buzzer.c"},
  {FILE_DISPLAY, "display.c"},
  {FILE_DRIVE_STATE_MANAGER, "drive_state_manager.c"},
  {FILE_LIGHT_SIGNAL_MANAGER, "light_signal_manager.c"},
  {FILE_PARTY_MODE, "party_mode.c"},
  {FILE_RANGE_ESTIMATOR, "range_estimator.c"},
  {FILE_STEERING, "steering.c"},
  {FILE_DATAGRAM, "datagram.c"},
  {FILE_SD_CARD_INTERFACE, "sd_card_interface.c"},
  {FILE_TELEMETRY, "telemetry.c"},
  {FILE_XB_TRANSMIT, "xb_transmit.c"}
};

/**
 * @brief Use to forward failures or continue on success.
 */
#define status_ok_or_return(code)          \
  ({                                       \
    __typeof__(code) status_expr = (code); \
    if (status_expr) return status_expr;   \
  })

  #define status_ok_or_warning(code)          \
  ({                                        \
    __typeof__(code) status_expr = (code);  \
    if (status_expr) {   \
      for (uint8_t i = 0; i < FILE_NUM_FILE; i++){\
        if (file_string_lut[i].name == __FILE__){\
          set_error_msg_status_code(status_expr); \
          set_error_msg_file(file_string_lut[i].file);\
        }\
      }                   \
       \
    }  \                              
  })
/** @} */
