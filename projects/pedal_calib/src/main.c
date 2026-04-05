/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for pedal_calib
 *
 * @date   2025-09-16
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "adc.h"
#include "dac.h"
#include "delay.h"
#include "flash.h"
#include "global_enums.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "opamp.h"
#include "persist.h"
#include "tasks.h"

/* Intra-component Headers */
#include "pedal_calib.h"

GpioAddress onboard_gpio = { .port = GPIO_PORT_B, .pin = 10 };
GpioAddress accel_pedal_gpio_opamp = { .port = GPIO_PORT_A, .pin = 3 };
GpioAddress accel_pedal_gpio_raw = { .port = GPIO_PORT_A, .pin = 0 };
GpioAddress brake_pedal_gpio = { .port = GPIO_PORT_A, .pin = 5 };

PedalPersistData pedal_persist_data = { 0U };
PedalCalibrationStorage pedal_storage = { 0U };
PersistStorage persist_storage = { 0U };

static StatusCode status;

#define LAST_PAGE (NUM_FLASH_PAGES - 1)

#define PEDAL_CALIB_DAC_CHANNEL DAC_CHANNEL1
#define PEDAL_CALIB_OPAMP OPAMP_1

TASK(pedal_calib, TASK_STACK_1024) {
  LOG_DEBUG("Pedal Calibration sequence...\r\n");

  flash_init();

  delay_ms(10U);
  if (sizeof(pedal_persist_data) % FLASH_MEMORY_ALIGNMENT != 0) {
    LOG_DEBUG("Warning: size of test struct: %u is not %u-byte aligned!!\r\n", sizeof(pedal_persist_data), FLASH_MEMORY_ALIGNMENT);
    delay_ms(10U);
  }

  status = flash_erase(NUM_FLASH_PAGES - 1U, 1U);
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("flash_erase() failed with exit code %u\r\n", status);
    delay_ms(10U);
  }

  status = persist_init(&persist_storage, LAST_PAGE, &pedal_persist_data, sizeof(pedal_persist_data), true);

  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("persist_init() failed with exit code %u\r\n", status);
    delay_ms(10U);
  }

  adc_add_channel(&accel_pedal_gpio_raw);
  adc_add_channel(&accel_pedal_gpio_opamp);
  adc_add_channel(&brake_pedal_gpio);

  adc_init();
  opamp_init();
  dac_init();
  gpio_init_pin(&onboard_gpio, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

  // accel pedal data collection & set to struct
  delay_ms(1000U);
  LOG_DEBUG("Lift the Accel Pedal all the way UP!\r\n");
  pedal_calib_sample(&pedal_storage, &(pedal_persist_data.accel_pedal_data_raw), PEDAL_UNPRESSED, &accel_pedal_gpio_raw);

  delay_ms(1000U);
  LOG_DEBUG("Push the Accel Pedal all the way DOWN!\r\n");
  pedal_calib_sample(&pedal_storage, &(pedal_persist_data.accel_pedal_data_raw), PEDAL_PRESSED, &accel_pedal_gpio_raw);

  // Initialize Opamp

  uint16_t min_raw_reading = (pedal_persist_data.accel_pedal_data_raw.lower_value < pedal_persist_data.accel_pedal_data_raw.upper_value) ? pedal_persist_data.accel_pedal_data_raw.lower_value
                                                                                                                                         : pedal_persist_data.accel_pedal_data_raw.upper_value;

  LOG_DEBUG("Setting MIN raw reading: %u\r\n", min_raw_reading);

  dac_enable_channel(PEDAL_CALIB_DAC_CHANNEL);
  dac_set_voltage(PEDAL_CALIB_DAC_CHANNEL, min_raw_reading);

  OpampConfig config = {
    .vinp_sel = OPAMP_NONINVERTING_IO0, /* PA1 - Pedal input */
    .vinm_sel = OPAMP_INVERTING_IO0,    /* PA0 - External feedback network */
    .output_to_adc = true               /* Flag for documentation */
  };

  opamp_configure(PEDAL_CALIB_OPAMP, &config);
  opamp_start(PEDAL_CALIB_OPAMP);

  delay_ms(1000U);
  LOG_DEBUG("Lift the Accel Pedal all the way UP!\r\n");
  pedal_calib_sample(&pedal_storage, &(pedal_persist_data.accel_pedal_data_amplified), PEDAL_UNPRESSED, &accel_pedal_gpio_opamp);

  delay_ms(1000U);
  LOG_DEBUG("Push the Accel Pedal all the way DOWN!\r\n");
  pedal_calib_sample(&pedal_storage, &(pedal_persist_data.accel_pedal_data_amplified), PEDAL_PRESSED, &accel_pedal_gpio_opamp);

  // brake pedal data collection & set to struct

  delay_ms(1000U);
  LOG_DEBUG("Lift the Brake Pedal all the way UP!\r\n");
  pedal_calib_sample(&pedal_storage, &(pedal_persist_data.brake_pedal_data), PEDAL_UNPRESSED, &brake_pedal_gpio);

  delay_ms(1000U);
  LOG_DEBUG("Push the Brake Pedal all the way DOWN!\r\n");
  pedal_calib_sample(&pedal_storage, &(pedal_persist_data.brake_pedal_data), PEDAL_PRESSED, &brake_pedal_gpio);

  status = persist_commit(&persist_storage);
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("persist_commit() failed with exit code %u\r\n", status);
    delay_ms(10U);
  }

  // Print values
  LOG_DEBUG("A Raw Max: %u | A Raw Min: %u\r\n", pedal_persist_data.accel_pedal_data_raw.upper_value, pedal_persist_data.accel_pedal_data_raw.lower_value);
  delay_ms(10U);
  LOG_DEBUG("A Max: %u | A Min: %u\r\n", pedal_persist_data.accel_pedal_data_amplified.upper_value, pedal_persist_data.accel_pedal_data_amplified.lower_value);
  delay_ms(10U);
  LOG_DEBUG("B Max: %u | B Min %u\r\n", pedal_persist_data.brake_pedal_data.upper_value, pedal_persist_data.brake_pedal_data.lower_value);
  delay_ms(10U);

  while (true) {
    // LOG_DEBUG("Accel Max: %u \r\n Accel Min: %u Accel Raw Min: %u \r\n Brake Max: %u \r\n Brake Min %u \r\n", pedal_persist_data.accel_pedal_data.upper_value,
    //           pedal_persist_data.accel_pedal_data.lower_value, pedal_persist_data.accel_pedal_data.opamp_offset, pedal_persist_data.brake_pedal_data.upper_value,
    //           pedal_persist_data.brake_pedal_data.lower_value);
    // delay_ms(1000U);
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

  tasks_init_task(pedal_calib, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
