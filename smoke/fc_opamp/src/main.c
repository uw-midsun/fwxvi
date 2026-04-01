/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for front_controller_pedal
 *
 * @date   2026-01-21
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "adc.h"
#include "delay.h"
#include "front_controller.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "power_manager.h"
#include "status.h"
#include "tasks.h"
#include "dac.h"
#include "opamp.h"

/* Intra-component Headers */
#include "accel_pedal.h"
#include "brake_pedal.h"
#include "front_controller_hw_defs.h"

FrontControllerStorage front_controller_storage = { 0 };

FrontControllerConfig front_controller_config = { .accel_input_deadzone = FRONT_CONTROLLER_ACCEL_INPUT_DEADZONE,
                                                  .accel_input_remap_min = FRONT_CONTROLLER_ACCEL_REMAP_MIN,
                                                  .accel_input_curve_exponent = FRONT_CONTROLLER_ACCEL_CURVE_EXPONENT,
                                                  .accel_low_pass_filter_alpha = FRONT_CONTROLLER_ACCEL_LPF_ALPHA,
                                                  .brake_pedal_deadzone = FRONT_CONTROLLER_BRAKE_INPUT_DEADZONE,
                                                  .brake_low_pass_filter_alpha = FRONT_CONTROLLER_BRAKE_LPF_ALPHA };

static GpioAddress s_accel_pedal_gpio_raw = GPIO_FRONT_CONTROLLER_ACCEL_PEDAL_RAW;
static GpioAddress s_accel_pedal_gpio_opamp_out = GPIO_FRONT_CONTROLLER_ACCEL_PEDAL_OPAMP_OUT;
static AccelPedalStorage s_accel_pedal_storage = { .calibration_data.opamp_offset = 583, .calibration_data.lower_value = 1154, .calibration_data.upper_value = 303 };
       
static uint16_t opamp_in_adc_reading;
static uint16_t opamp_out_adc_reading;
static float calculated_gain;

TASK(run_pedal, TASK_STACK_1024) {
  // Step 1: initialize components
  StatusCode status = STATUS_CODE_OK;

  adc_add_channel(&s_accel_pedal_gpio_raw);
  adc_add_channel(&s_accel_pedal_gpio_opamp_out);

  dac_enable_channel(FRONT_CONTROLLER_ACCEL_PEDAL_OPAMP_VREF_DAC);
  dac_set_voltage(FRONT_CONTROLLER_ACCEL_PEDAL_OPAMP_VREF_DAC, s_accel_pedal_storage.calibration_data.opamp_offset);

  OpampConfig config = {
    .vinp_sel = OPAMP_NONINVERTING_IO0,    /* PA1 - Pedal input */
    .vinm_sel = OPAMP_INVERTING_IO0,       /* PA0 - External feedback network */
    .output_to_adc = true                  /* Flag for documentation */
  };

  opamp_configure(FRONT_CONTROLLER_ACCEL_PEDAL_OPAMP, &config);
  opamp_start(FRONT_CONTROLLER_ACCEL_PEDAL_OPAMP);

  if (s_accel_pedal_storage.calibration_data.lower_value > s_accel_pedal_storage.calibration_data.upper_value) {
    uint16_t tmp = s_accel_pedal_storage.calibration_data.lower_value;
    s_accel_pedal_storage.calibration_data.lower_value = s_accel_pedal_storage.calibration_data.upper_value;
    s_accel_pedal_storage.calibration_data.upper_value = tmp;
    s_accel_pedal_storage.calibration_data.reversed = true;
  }

  adc_init();

  // Step 2: Run pedals
  while (true) {
    adc_run();

    adc_read_raw(&s_accel_pedal_gpio_raw, &opamp_in_adc_reading);
    adc_read_raw(&s_accel_pedal_gpio_opamp_out, &opamp_out_adc_reading);

    calculated_gain = (float)((float)opamp_out_adc_reading / (float)(opamp_in_adc_reading - s_accel_pedal_storage.calibration_data.opamp_offset));

    delay_ms(10U);
  }
}

TASK(display_pedal_stats, TASK_STACK_1024) {
  delay_ms(1000U);

  while (true) {
    printf("OPAMP IN: %d | OPAMP_OUT: %d | GAIN: %d.%d\r\n", opamp_in_adc_reading, opamp_out_adc_reading, (uint16_t)calculated_gain, ((uint16_t)calculated_gain * 100) % 100);
    delay_ms(100U);
  }
}

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
int main(int argc, char *argv[]) {
  mpxe_init(argc, argv);
#else
int main() {
#endif
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(run_pedal, TASK_PRIORITY(3), NULL);
  tasks_init_task(display_pedal_stats, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
