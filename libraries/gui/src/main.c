/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for GUI
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */
#include "gui.h"

void pre_loop_init() {}

void run_1000hz_cycle() {}

void run_10hz_cycle() {}

void run_1hz_cycle() {}

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

  init_master_tasks();

  tasks_start();

    // Just playing around for now
    ltdc_init();

    // Allocate test framebuffer and clut (will be defined more clearly by Dhyey)
    static uint8_t framebuffer[480 * 272];
    static uint32_t clut[256];

    // Fill framebuffer with a gradient (for demo)
    for (int i = 0; i < (480 * 272); i++) {
        framebuffer[i] = i % 256;
    }

    // Fill CLUT with a blue-green gradient
    for (int i = 0; i < 256; i++) {
        clut[i] = (i << 8) | (255 - i);  // RGB = 0x00GGrr
    }

    ltdc_draw();

    ltdc_destroy();

  LOG_DEBUG("exiting main?");

  return 0;
}
