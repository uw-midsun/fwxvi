/************************************************************************************************
 * @file    main.c
 *
 * @brief   Permanent first stage, validates the bootloader image and jumps to it
 *
 * @date    2026-06-04
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "stm32l4xx.h"

/* Intra-component Headers */
#include "bl_bootstrap.h"

extern uint32_t _bootstrap_start;

int main(void) {
  SCB->VTOR = (uint32_t)(uintptr_t)&_bootstrap_start;

  /* A successful jump never returns, a returned error means the bootloader is not runnable,
     halt so SWD can reflash it (the bootstrap itself is never reflashed over the bus) */
  bl_bootstrap_run();

  while (true) {
  }
  return 0;
}
