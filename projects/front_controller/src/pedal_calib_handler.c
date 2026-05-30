/************************************************************************************************
 * @file   pedal_calib_handler.c
 *
 * @brief  Handler for remote pedal calibration via CAN
 *
 * @date   2026-04-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "adc.h"
#include "dac.h"
#include "delay.h"
#include "flash.h"
#include "front_controller_getters.h"
#include "front_controller_setters.h"
#include "gpio.h"
#include "log.h"
#include "opamp.h"
#include "pedal_calib.h"
#include "persist.h"
#include "status.h"
#include "task.h"

/* Intra-component Headers */
#include "front_controller.h"
#include "front_controller_hw_defs.h"
#include "pedal_calib_handler.h"

/**
 * @brief State machine for pedal calibration
 */
typedef struct {
  bool active;
  PedalCalibStatus current_step;
  uint32_t step_start_time_ms;
  PedalCalibrationStorage calib_storage;
  PedalPersistData pedal_persist_data;
  PersistStorage persist_storage;
} PedalCalibrationState;

static PedalCalibrationState s_calib_state = { 0 };
static bool command_active = false;

/**
 * @brief   Get current time in milliseconds
 * @return  Current time in milliseconds
 */
static uint32_t s_get_current_time_ms(void) {
  return (uint32_t)(xTaskGetTickCount() * 1000 / configTICK_RATE_HZ);
}

// GPIO addresses for pedal inputs
static const GpioAddress s_accel_pedal_gpio_opamp = GPIO_FRONT_CONTROLLER_ACCEL_PEDAL_OPAMP_OUT;
static const GpioAddress s_accel_pedal_gpio_raw = GPIO_FRONT_CONTROLLER_ACCEL_PEDAL_RAW;
static const GpioAddress s_brake_pedal_gpio = GPIO_FRONT_CONTROLLER_BRAKE_PEDAL;

// Hardware configuration
#define LAST_PAGE (NUM_FLASH_PAGES - 1)
#define PEDAL_CALIB_DAC_CHANNEL DAC_CHANNEL1
#define PEDAL_CALIB_OPAMP OPAMP_1
#define DELAY_BEFORE_SAMPLING_MS 1000U

/**
 * @brief   Send calibration status via CAN to steering
 */
static StatusCode s_send_calib_status(PedalCalibStatus status) {
  set_pedal_calib_status_status(status);
  LOG_DEBUG("Pedal Calib Status: %u\r\n", status);
  return STATUS_CODE_OK;
}

/**
 * @brief   Execute one step of the calibration
 */
static StatusCode s_execute_calib_step(void) {
  uint32_t elapsed_time = s_get_current_time_ms();  // Get current time

  switch (s_calib_state.current_step) {
    case PEDAL_CALIB_STATUS_IDLE:
      adc_add_channel(&s_accel_pedal_gpio_raw);
      s_calib_state.current_step = PEDAL_CALIB_STATUS_ACCEL_RAW_UNPRESSED;
      s_calib_state.step_start_time_ms = s_get_current_time_ms();
      s_send_calib_status(PEDAL_CALIB_STATUS_ACCEL_RAW_UNPRESSED);
      break;

    case PEDAL_CALIB_STATUS_ACCEL_RAW_UNPRESSED:
      if (s_get_current_time_ms() - s_calib_state.step_start_time_ms >= DELAY_BEFORE_SAMPLING_MS) {
        LOG_DEBUG("Sampling accel pedal raw (unpressed)\r\n");
        pedal_calib_sample(&s_calib_state.calib_storage, &s_calib_state.pedal_persist_data.accel_pedal_data_raw, PEDAL_UNPRESSED, (GpioAddress *)&s_accel_pedal_gpio_raw);
        s_calib_state.current_step = PEDAL_CALIB_STATUS_ACCEL_RAW_PRESSED;
        s_calib_state.step_start_time_ms = s_get_current_time_ms();
        s_send_calib_status(PEDAL_CALIB_STATUS_ACCEL_RAW_PRESSED);
      }
      break;

    case PEDAL_CALIB_STATUS_ACCEL_RAW_PRESSED:
      if (s_get_current_time_ms() - s_calib_state.step_start_time_ms >= DELAY_BEFORE_SAMPLING_MS) {
        LOG_DEBUG("Sampling accel pedal raw (pressed)\r\n");
        pedal_calib_sample(&s_calib_state.calib_storage, &s_calib_state.pedal_persist_data.accel_pedal_data_raw, PEDAL_PRESSED, (GpioAddress *)&s_accel_pedal_gpio_raw);

        // Setup OPAMP for next phase
        uint16_t min_raw_reading = (s_calib_state.pedal_persist_data.accel_pedal_data_raw.lower_value < s_calib_state.pedal_persist_data.accel_pedal_data_raw.upper_value)
                                       ? s_calib_state.pedal_persist_data.accel_pedal_data_raw.lower_value
                                       : s_calib_state.pedal_persist_data.accel_pedal_data_raw.upper_value;

        LOG_DEBUG("Setting MIN raw reading: %u\r\n", min_raw_reading);

        dac_enable_channel(PEDAL_CALIB_DAC_CHANNEL);
        dac_set_voltage(PEDAL_CALIB_DAC_CHANNEL, min_raw_reading);

        OpampConfig config = { .vinp_sel = OPAMP_NONINVERTING_IO0, .vinm_sel = OPAMP_INVERTING_IO0, .output_to_adc = true };

        opamp_configure(PEDAL_CALIB_OPAMP, &config);
        opamp_start(PEDAL_CALIB_OPAMP);

        s_calib_state.current_step = PEDAL_CALIB_STATUS_ACCEL_AMP_UNPRESSED;
        s_calib_state.step_start_time_ms = s_get_current_time_ms();
        s_send_calib_status(PEDAL_CALIB_STATUS_ACCEL_AMP_UNPRESSED);
      }
      break;

    case PEDAL_CALIB_STATUS_ACCEL_AMP_UNPRESSED:
      if (s_get_current_time_ms() - s_calib_state.step_start_time_ms >= DELAY_BEFORE_SAMPLING_MS) {
        LOG_DEBUG("Sampling accel pedal amplified (unpressed)\r\n");
        pedal_calib_sample(&s_calib_state.calib_storage, &s_calib_state.pedal_persist_data.accel_pedal_data_amplified, PEDAL_UNPRESSED, (GpioAddress *)&s_accel_pedal_gpio_opamp);
        s_calib_state.current_step = PEDAL_CALIB_STATUS_ACCEL_AMP_PRESSED;
        s_calib_state.step_start_time_ms = s_get_current_time_ms();
        s_send_calib_status(PEDAL_CALIB_STATUS_ACCEL_AMP_PRESSED);
      }
      break;

    case PEDAL_CALIB_STATUS_ACCEL_AMP_PRESSED:
      if (s_get_current_time_ms() - s_calib_state.step_start_time_ms >= DELAY_BEFORE_SAMPLING_MS) {
        LOG_DEBUG("Sampling accel pedal amplified (pressed)\r\n");
        pedal_calib_sample(&s_calib_state.calib_storage, &s_calib_state.pedal_persist_data.accel_pedal_data_amplified, PEDAL_PRESSED, (GpioAddress *)&s_accel_pedal_gpio_opamp);
        s_calib_state.current_step = PEDAL_CALIB_STATUS_BRAKE_UNPRESSED;
        s_calib_state.step_start_time_ms = s_get_current_time_ms();
        s_send_calib_status(PEDAL_CALIB_STATUS_BRAKE_UNPRESSED);
      }
      break;

    case PEDAL_CALIB_STATUS_BRAKE_UNPRESSED:
      if (s_get_current_time_ms() - s_calib_state.step_start_time_ms >= DELAY_BEFORE_SAMPLING_MS) {
        LOG_DEBUG("Sampling brake pedal (unpressed)\r\n");
        pedal_calib_sample(&s_calib_state.calib_storage, &s_calib_state.pedal_persist_data.brake_pedal_data, PEDAL_UNPRESSED, (GpioAddress *)&s_brake_pedal_gpio);
        s_calib_state.current_step = PEDAL_CALIB_STATUS_BRAKE_PRESSED;
        s_calib_state.step_start_time_ms = s_get_current_time_ms();
        s_send_calib_status(PEDAL_CALIB_STATUS_BRAKE_PRESSED);
      }
      break;

    case PEDAL_CALIB_STATUS_BRAKE_PRESSED:
      if (s_get_current_time_ms() - s_calib_state.step_start_time_ms >= DELAY_BEFORE_SAMPLING_MS) {
        LOG_DEBUG("Sampling brake pedal (pressed)\r\n");
        pedal_calib_sample(&s_calib_state.calib_storage, &s_calib_state.pedal_persist_data.brake_pedal_data, PEDAL_PRESSED, (GpioAddress *)&s_brake_pedal_gpio);

        // Commit to persist storage
        StatusCode status = persist_commit(&s_calib_state.persist_storage);
        if (status != STATUS_CODE_OK) {
          LOG_DEBUG("persist_commit() failed with status %u\r\n", status);
          s_calib_state.current_step = PEDAL_CALIB_STATUS_ERROR;
          s_send_calib_status(PEDAL_CALIB_STATUS_ERROR);
        } else {
          LOG_DEBUG("Pedal calibration complete!\r\n");
          s_calib_state.current_step = PEDAL_CALIB_STATUS_COMPLETE;
          s_send_calib_status(PEDAL_CALIB_STATUS_COMPLETE);
        }
      }
      break;

    case PEDAL_CALIB_STATUS_COMPLETE:
    case PEDAL_CALIB_STATUS_ERROR:
      // Calibration is done, will be reset by next init
      break;

    default:
      return STATUS_CODE_UNKNOWN;
  }

  return STATUS_CODE_OK;
}

StatusCode pedal_calib_handler_init(FrontControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_calib_state.active = false;
  s_calib_state.current_step = PEDAL_CALIB_STATUS_IDLE;
  s_calib_state.calib_storage = (PedalCalibrationStorage){ 0 };
  s_calib_state.pedal_persist_data = (PedalPersistData){ 0 };

  return STATUS_CODE_OK;
}

StatusCode pedal_calib_handler_start(FrontControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  // Activate the calibration state machine
  if (!s_calib_state.active) {
    s_calib_state.active = true;
    s_calib_state.current_step = PEDAL_CALIB_STATUS_IDLE;
    LOG_DEBUG("Pedal calibration started\r\n");
  }

  return STATUS_CODE_OK;
}

StatusCode pedal_calib_handler_run(FrontControllerStorage *storage) {
  LOG_DEBUG("%d | %d\r\n", command_active, get_pedal_calib_request_command());

  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  command_active = get_pedal_calib_request_command() != 0;

  if (!s_calib_state.active) {
    if (command_active) {
      status_ok_or_return(pedal_calib_handler_start(storage));
    }
    return STATUS_CODE_OK;
  }

  // Allow re-run after completion once steering clears the command
  if (!command_active && (s_calib_state.current_step == PEDAL_CALIB_STATUS_COMPLETE || s_calib_state.current_step == PEDAL_CALIB_STATUS_ERROR)) {
    LOG_DEBUG("Resetting\r\n");
    s_calib_state.active = false;
    s_calib_state.current_step = PEDAL_CALIB_STATUS_IDLE;
    return STATUS_CODE_OK;
  }
  
  LOG_DEBUG("executing step\r\n");
  return s_execute_calib_step();
}
