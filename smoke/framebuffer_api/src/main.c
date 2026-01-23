/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for framebuffer_api
 *
 * @date   2026-01-22
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "status.h"
#include "delay.h"
#include "framebuffer.h"


/* Intra-component Headers */

const uint16_t WIDTH = 480; 
const uint16_t HEIGHT = 272;  

static uint8_t framebuffer[WIDTH * HEIGHT];

Framebuffer framebuffer_config = {0}; 

TASK(framebuffer_api, TASK_STACK_1024) {
  StatusCode status = STATUS_CODE_OK;

  status = framebuffer_init(&framebuffer_config, WIDTH, HEIGHT, &framebuffer);
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("framebuffer initialized\r\n");
  } else {
    LOG_DEBUG("framebuffer cannot be initialized\r\n");
  }

  while (true) {

    printf("Testing framebuffer_set_pixels of current framebuffer to white\n"); 

    uint16_t i, j; 

    for (i = 0; i < WIDTH; i++) {
      for (i = 0; i < HEIGHT; i++) {
        status = framebuffer_set_pixel(&framebuffer_config,  i, j, COLOR_INDEX_WHITE); 

        if (status != STATUS_CODE_OK){
          LOG_DEBUG("Setting frame pixel at %d, %d, to white failed \n", i, j);
        } else {
          printf("Setting frame pixel at %d, %d, suceeded \n", i, j);
        }
      }
    }

    printf("Testing framebuffer_get_pixels\n");
    
    for (i = 0; i < WIDTH; i++) {
      for (i = 0; i < HEIGHT; i++) {
        status = framebuffer_get_pixel(&framebuffer_config,  i, j); 
        if (status != STATUS_CODE_OK){
          LOG_DEBUG("Getting frame pixel at %d, %d failed \n", i, j);
        } else {
          printf("Getting frame pixel at %d, %d suceeded \n", i, j);
        }
      }
    }

    printf("Testing framebuffer_clear to white"); 
    status = framebuffer_clear(&framebuffer_config, COLOR_INDEX_WHITE); 

    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("framebuffer_clear to white failed \r\n");
    } else {
      printf("framebuffer_clear succeeded \n");
    }

    delay_ms(1000);
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
  
  tasks_init_task(framebuffer_api, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}