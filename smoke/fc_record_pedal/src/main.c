/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for pedal calibration recording and reading
 *
 * @date   2026-02-07
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "adc.h"
#include "delay.h"
#include "flash.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "persist.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */
#include "front_controller_hw_defs.h"

// Mode switching - change this line to switch between modes
#define RECORD_MODE 0U
#define READ_MODE 1U
#define CALIBRATION_MODE READ_MODE

#define LAST_PAGE (NUM_FLASH_PAGES - 1)

// Calibration sampling parameters
#define CALIBRATION_SAMPLES 100U
#define CALIBRATION_DELAY_MS 10U
#define CALIBRATION_FILTER_ALPHA 0.3f

// Data structure - must match pedal_calib_reader.c format
typedef struct PedalCalibrationData {
  int16_t lower_value;  // Pedal fully released (unpressed)
  int16_t upper_value;  // Pedal fully pressed
} PedalCalibrationData;

typedef struct __attribute__((aligned(4))) PedalPersistData {
  PedalCalibrationData accel_pedal_data;
  PedalCalibrationData brake_pedal_data;
} PedalPersistData;

PedalPersistData persist_data = { 0 };
PersistStorage persist_storage = { 0 };

// Samples pedal with smoothing
StatusCode sample_pedal_calibration(GpioAddress *pedal_gpio, int16_t *result_value) {
  uint16_t adc_reading = 0;
  float smoothed_value = 0.0f;
  bool first_sample = true;

  for (uint32_t i = 0; i < CALIBRATION_SAMPLES; i++) {
    adc_run();
    adc_read_raw(pedal_gpio, &adc_reading);

    if (first_sample) {
      smoothed_value = (float)adc_reading;
      first_sample = false;
    } else {
      smoothed_value = CALIBRATION_FILTER_ALPHA * (float)adc_reading + (1.0f - CALIBRATION_FILTER_ALPHA) * smoothed_value;
    }

    delay_ms(CALIBRATION_DELAY_MS);
  }

  *result_value = (int16_t)(smoothed_value + 0.5f);
  return STATUS_CODE_OK;
}

#if (CALIBRATION_MODE == RECORD_MODE)

TASK(record_pedal_calibration, TASK_STACK_1024) {
  StatusCode status = STATUS_CODE_OK;

  flash_init();

  status = persist_init(&persist_storage, LAST_PAGE, &persist_data, sizeof(persist_data), true);
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("persist_init() failed with exit code %u\r\n", status);
    while (true) {
    }
  }

  gpio_init_pin(&GPIO_FRONT_CONTROLLER_ACCEL_PEDAL, GPIO_ANALOG, GPIO_STATE_LOW);
  gpio_init_pin(&GPIO_FRONT_CONTROLLER_BRAKE_PEDAL, GPIO_ANALOG, GPIO_STATE_LOW);
  adc_add_channel(&GPIO_FRONT_CONTROLLER_ACCEL_PEDAL);
  adc_add_channel(&GPIO_FRONT_CONTROLLER_BRAKE_PEDAL);

  // Accel pedal calibration
  LOG_DEBUG("LIFT accelerator pedal ALL THE WAY UP\r\n");
  delay_ms(3000);
  sample_pedal_calibration(&GPIO_FRONT_CONTROLLER_ACCEL_PEDAL, &persist_data.accel_pedal_data.lower_value);
  LOG_DEBUG("Accel minimum: %d\r\n", persist_data.accel_pedal_data.lower_value);

  LOG_DEBUG("PUSH accelerator pedal ALL THE WAY DOWN\r\n");
  delay_ms(3000);
  sample_pedal_calibration(&GPIO_FRONT_CONTROLLER_ACCEL_PEDAL, &persist_data.accel_pedal_data.upper_value);
  LOG_DEBUG("Accel maximum: %d\r\n", persist_data.accel_pedal_data.upper_value);

  // Brake pedal calibration
  LOG_DEBUG("LIFT brake pedal ALL THE WAY UP\r\n");
  delay_ms(3000);
  sample_pedal_calibration(&GPIO_FRONT_CONTROLLER_BRAKE_PEDAL, &persist_data.brake_pedal_data.lower_value);
  LOG_DEBUG("Brake minimum: %d\r\n", persist_data.brake_pedal_data.lower_value);

  LOG_DEBUG("PUSH brake pedal ALL THE WAY DOWN\r\n");
  delay_ms(3000);
  sample_pedal_calibration(&GPIO_FRONT_CONTROLLER_BRAKE_PEDAL, &persist_data.brake_pedal_data.upper_value);
  LOG_DEBUG("Brake maximum: %d\r\n", persist_data.brake_pedal_data.upper_value);

  // Write to persist memory
  status = persist_commit(&persist_storage);
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("SUCCESS! Calibration saved.\r\n");
  }

  while (true) {
    LOG_DEBUG("fc_record_pedal: calibration complete, task idle\r\n")
    delay_ms(2000);
  }
}

#else

TASK(read_pedal_calibration, TASK_STACK_1024) {
  StatusCode status = STATUS_CODE_OK;

  flash_init();

  status = persist_init(&persist_storage, LAST_PAGE, &persist_data, sizeof(persist_data), false);
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("persist_init() failed with exit code %u\r\n", status);
    while (true) {
    }
  }

  while (true) {
    LOG_DEBUG("Accel: %d to %d\r\n", persist_data.accel_pedal_data.lower_value, persist_data.accel_pedal_data.upper_value);
    LOG_DEBUG("Brake: %d to %d\r\n", persist_data.brake_pedal_data.lower_value, persist_data.brake_pedal_data.upper_value);
    delay_ms(2000);
  }
}

#endif

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

#if (CALIBRATION_MODE == RECORD_MODE)
  tasks_init_task(record_pedal_calibration, TASK_PRIORITY(3), NULL);
#else
  tasks_init_task(read_pedal_calibration, TASK_PRIORITY(3), NULL);
#endif

  tasks_start();
  return 0;
}
