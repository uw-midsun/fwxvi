/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for front_controller_pedal_calib_read
 *
 * @date   2025-09-16
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdio.h>

/* Inter-component Headers */
#include "delay.h"
#include "flash.h"
#include "global_enums.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "persist.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */

#define PERSIST_WRITER_MODE 0U
#define PERSIST_READER_MODE 1U
#define LAST_PAGE (NUM_FLASH_PAGES - 1)

/*************************************** SET SMOKE TEST MODE ************************************** */
#define PERSIST_MODE PERSIST_READER_MODE
/************************************************************************************************** */

PedalPersistData test_struct = { 0U };
PersistStorage storage = { 0U };

TASK(persist_api, TASK_STACK_1024) {
  StatusCode status = STATUS_CODE_OK;

  flash_init();
#if (PERSIST_MODE == PERSIST_WRITER_MODE)

  LOG_DEBUG("Starting pedal calib smoke test - writer mode\r\n");
  delay_ms(10U);
  if (sizeof(test_struct) % FLASH_MEMORY_ALIGNMENT != 0) {
    LOG_DEBUG("Warning: size of test struct: %u is not %u-byte aligned!!\r\n", sizeof(test_struct), FLASH_MEMORY_ALIGNMENT);
    delay_ms(10U);
  }

  status = flash_erase(NUM_FLASH_PAGES - 1U, 1U);
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("flash_erase() failed with exit code %u\r\n", status);
    delay_ms(10U);
  }

  status = persist_init(&storage, LAST_PAGE, &test_struct, sizeof(test_struct), true);

  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("persist_init() failed with exit code %u\r\n", status);
    delay_ms(10U);
  }

  test_struct.accel_pedal_data_amplified.lower_value = 1U;
  test_struct.accel_pedal_data_amplified.upper_value = 2U;
  test_struct.accel_pedal_data_raw.lower_value = 3U;
  test_struct.accel_pedal_data_raw.upper_value = 4U;
  test_struct.brake_pedal_data.lower_value = 5U;
  test_struct.brake_pedal_data.upper_value = 6U;

  status = persist_commit(&storage);
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("persist_commit() failed with exit code %u\r\n", status);
    delay_ms(10U);
  }

  LOG_DEBUG("Writer commited.\r\n");
  delay_ms(10U);
  LOG_DEBUG("Wrote accel raw: min=%u max=%u | accel amp: min=%u max=%u | brake: min=%u max=%u\r\n", test_struct.accel_pedal_data_raw.lower_value, test_struct.accel_pedal_data_raw.upper_value,
            test_struct.accel_pedal_data_amplified.lower_value, test_struct.accel_pedal_data_amplified.upper_value, test_struct.brake_pedal_data.lower_value, test_struct.brake_pedal_data.upper_value);
  delay_ms(10U);

#else
  LOG_DEBUG("Starting pedal calib smoke test - reader mode\r\n");
  delay_ms(10U);
  status = persist_init(&storage, LAST_PAGE, &test_struct, sizeof(test_struct), false);
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("persist_init() failed with exit code %u\r\n", status);
    delay_ms(10U);
  }
  LOG_DEBUG("Read accel raw: min=%u max=%u | accel amp: min=%u max=%u | brake: min=%u max=%u\r\n", test_struct.accel_pedal_data_raw.lower_value, test_struct.accel_pedal_data_raw.upper_value,
            test_struct.accel_pedal_data_amplified.lower_value, test_struct.accel_pedal_data_amplified.upper_value, test_struct.brake_pedal_data.lower_value, test_struct.brake_pedal_data.upper_value);

#endif

  while (true) {
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

  tasks_init_task(persist_api, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
