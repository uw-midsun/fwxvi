/************************************************************************************************
 * @file   pedal.c
 *
 * @brief  Main file for centre_console
 *
 * @date   2025-01-30
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "cc_hw_defs.h"
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */
#include "centre_console_setters.h"
#include "pedal.h"

static const GpioAddress brake = BRAKE_LIMIT_SWITCH;
static const GpioAddress throttle = ADC_HALL_SENSOR;

// TODO: Implement the calibrate file
static PedalCalibBlob *s_calib_blob;

static void s_read_throttle_data(uint32_t *reading) {
  volatile uint16_t adc_reading = s_calib_blob->throttle_calib.lower_value;
  // adc_read_raw(&throttle, &adc_reading);

  // Convert ADC Reading to readable voltage by normalizing with calibration data and dividing
  // to get percentage press. Brake is now just a GPIO. Negatives and > 100 values will be
  // capped.
  // In this case, the values are mapped to 0.2 to 1
  volatile float calculated_reading =
      ((((float)adc_reading - (float)s_calib_blob->throttle_calib.lower_value) / ((float)s_calib_blob->throttle_calib.upper_value - (float)s_calib_blob->throttle_calib.lower_value)) * 0.8 + 0.2);

  // Readings are inverted but we have to account for mapping from 0.2 to 1
  calculated_reading = 1.2 - calculated_reading;

  // Clamp the reading and make sure it stays within range
  if (calculated_reading < 0.2) {
    calculated_reading = 0;
  } else if (calculated_reading > 1) {
    calculated_reading = 1;
  }
  memcpy(reading, &calculated_reading, sizeof(calculated_reading));
}

// TODO: Add a high pass filter that smoothens out high frequency data
void pedal_run() {
  GpioState brake_state = GPIO_STATE_LOW;
  brake_state = gpio_get_state(&brake);
  uint32_t throttle_position = 0;
  s_read_throttle_data(&throttle_position);

  if (brake_state == GPIO_STATE_LOW) {
    set_cc_pedal_brake_output(0);
    set_cc_pedal_throttle_output(throttle_position);
  } else {
    set_cc_pedal_brake_output(1);
    set_cc_pedal_throttle_output(0);
  }
}

StatusCode pedal_init(PedalCalibBlob *calib_blob) {
  // Set pointers to static variables that will be provided to all pedal files
  interrupt_init();
  gpio_init_pin(&brake, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  gpio_init_pin(&throttle, GPIO_ANALOG, GPIO_STATE_LOW);
  // adc_add_channel(throttle);
  s_calib_blob = calib_blob;
  return STATUS_CODE_OK;
}
