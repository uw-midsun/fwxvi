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
<<<<<<< HEAD
#include "network.h"
#include "network_buffer.h"
#include "fota_jump_app.h"
=======
#include "fota.h"
#include "fota_jump_handler.h"
>>>>>>> 34254f5b500c64ec4e3906266eadb12cb93b2a2d
#include "fota_timeout.h"

int main() {
  fota_init();

  while (true) {
    if (fota_is_timed_out()) {
      fota_jump(FOTA_JUMP_APPLICATION);
    }
<<<<<<< HEAD
    if (fota_is_timed_out()) {
      bootloader_jump_app();
    }
=======

    fota_process();
>>>>>>> 34254f5b500c64ec4e3906266eadb12cb93b2a2d
  }

  return 0;
}
