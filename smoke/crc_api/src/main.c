/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for crc_api
 *
 * @date   2025-02-05
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "crc.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */

#define TEST_NUM_CRC32_WORDS 3U
#define TEST_NUM_CRC16_WORDS 4U
#define TEST_NUM_CRC8_WORDS 6U

TASK(crc_api, TASK_STACK_1024) {
  uint32_t crc32_array[TEST_NUM_CRC32_WORDS] = { 0xDEADBEEF, 0xCAFEDEED, 0x12345678 };
  uint16_t crc16_array[TEST_NUM_CRC16_WORDS] = { 0xDEAD, 0xBEEF, 0xCAFE, 0xDEED };
  uint8_t crc8_array[TEST_NUM_CRC8_WORDS] = { 1, 2, 3, 4, 5, 6 };

  while (true) {
    crc_init(CRC_LENGTH_32);
    delay_ms(1000);
    LOG_DEBUG("CRC32 Calculation: %lu\n", (uint32_t)crc_calculate(crc32_array, TEST_NUM_CRC32_WORDS));

    crc_init(CRC_LENGTH_16);
    delay_ms(1000);
    LOG_DEBUG("CRC16 Calculation: %lu\n", (uint32_t)crc_calculate((uint32_t *)crc16_array, TEST_NUM_CRC16_WORDS));

    crc_init(CRC_LENGTH_8);
    delay_ms(1000);
    LOG_DEBUG("CRC8 Calculation: %lu\n", (uint32_t)crc_calculate((uint32_t *)crc8_array, TEST_NUM_CRC8_WORDS));

    delay_ms(10);
    LOG_DEBUG("Done calculations! Run the python CRC script (python3 smoke/crc_api/scripts/crc.py) to verify these values!\n\n");
  }
}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(crc_api, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
