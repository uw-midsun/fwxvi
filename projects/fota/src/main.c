/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for fota
 *
 * @date   2025-03-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include <stdbool.h>
// #include "log.h"

/* Intra-component Headers */
#include "fota.h"
#include "network.h"
#include "network_buffer.h"

int main() {
  fota_init();

  while (true) {
    if (isTimeout(true)) {
      // tx timeout, rx timeout is built into function to return error if timeout
    }
    if (fota_is_timed_out()) {
      bootloader_jump_app();
    }

    fota_process();
  }

  return 0;
}
