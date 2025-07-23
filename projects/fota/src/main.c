/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for fota
 *
 * @date   2025-03-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota.h"
#include "fota_jump_handler.h"
#include "fota_timeout.h"

int main() {
  fota_init();

  while (true) {
    if (fota_is_timed_out()) {
      fota_jump(FOTA_JUMP_APPLICATION);
    }

    fota_process();
  }

  return 0;
}
