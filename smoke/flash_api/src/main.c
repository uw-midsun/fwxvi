/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for [PROJECT NAME]
 *
 * @date   [YYYY/MM/DD]
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "flash.h"
#include "delay.h"

/* Intra-component Headers */

TASK(flash_api_test, TASK_STACK_2048) {
  StatusCode status = STATUS_CODE_OK;
  uint8_t flash_buffer[FLASH_PAGE_SIZE] = {0U};
  while (true) {
    LOG_DEBUG("Running Flash Smoke!\n");

    do {
      /* Erase flash memory so it is cleared */
      status = flash_erase(NUM_FLASH_PAGES - 1U, 1U);

      if (status != STATUS_CODE_OK) {
        break;
      }

      /* Update flash buffer */
      for (size_t i = 0U; i < sizeof(flash_buffer); i++) {
        flash_buffer[i] = i % 256;
      }

      status = flash_write(FLASH_PAGE_TO_ADDR(NUM_FLASH_PAGES - 1U), flash_buffer, sizeof(flash_buffer));

      if (status != STATUS_CODE_OK) {
        break;
      }

      status = flash_read(FLASH_PAGE_TO_ADDR(NUM_FLASH_PAGES - 1U), flash_buffer, sizeof(flash_buffer));

      if (status != STATUS_CODE_OK) {
        break;
      }

      for (size_t i = 0U; i < sizeof(flash_buffer); i++) {
        if (flash_buffer[i] != (i % 256)) {
          LOG_DEBUG("Mismatch in expected data %d and actual data %d\n", i, flash_buffer[i]);
          delay_ms(1);
        }
      }
    } while (false);

    LOG_DEBUG("Done test! Exited with status code %d\n", status);
    status = STATUS_CODE_OK;
    delay_ms(500);
  }
}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(flash_api_test, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
