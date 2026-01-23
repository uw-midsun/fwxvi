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
#include "gui.h"
#include "clut.h"
#include "ltdc.h"


/* Intra-component Headers */

const uint16_t WIDTH = 480; 
const uint16_t HEIGHT = 272;  
static uint8_t framebuffer[WIDTH * HEIGHT] __attribute__((aligned(32)));
static GpioAddress s_display_ctrl = { .port = GPIO_PORT_A, .pin = 0 };
static GpioAddress s_display_current_ctrl = { .port = GPIO_PORT_A, .pin = 1 };
static LtdcSettings settings = { 0 };

StatusCode draw_checkerboard(ColorIndex color1, ColorIndex color2, uint16_t square_size) {
  for (uint16_t y = 0; y < HEIGHT; y++) {
    for (uint16_t x = 0; x < WIDTH; x++) {
      uint16_t square_x = x / square_size;
      uint16_t square_y = y / square_size;
      ColorIndex color = ((square_x + square_y) % 2 == 0) ? color1 : color2;
      status_ok_or_return(ltdc_set_pixel(x, y, color));
    }
  }
  /* Apply changes */
  return ltdc_draw();
}

TASK(framebuffer_api, TASK_STACK_1024) {
  LtdcTimingConfig timing_config = { .hsync = 4, .vsync = 4, .hbp = 43, .vbp = 12, .hfp = 8, .vfp = 8 };

  LtdcGpioConfig gpio_config = {
    .clk = {.port = GPIO_PORT_A, .pin = 4},
    .hsync = {.port = GPIO_PORT_C, .pin = 2},
    .vsync = {.port = GPIO_PORT_B, .pin = 11},
    .de = {.port = GPIO_PORT_C, .pin = 0},
    .r = {
      {},
      {},
      {.port = GPIO_PORT_E, .pin = 15},  /* R2 */
      {.port = GPIO_PORT_D, .pin = 8},   /* R3 */
      {.port = GPIO_PORT_D, .pin = 9},   /* R4 */
      {.port = GPIO_PORT_D, .pin = 10},  /* R5 */
      {.port = GPIO_PORT_D, .pin = 11},  /* R6 */
      {.port = GPIO_PORT_D, .pin = 12},  /* R7 */
    },
    .g = {
      {},
      {},
      {.port = GPIO_PORT_E, .pin = 9},  /* G2 */
      {.port = GPIO_PORT_E, .pin = 10}, /* G3 */
      {.port = GPIO_PORT_E, .pin = 11}, /* G4 */
      {.port = GPIO_PORT_E, .pin = 12}, /* G5 */
      {.port = GPIO_PORT_E, .pin = 13}, /* G6 */
      {.port = GPIO_PORT_E, .pin = 14}, /* G7 */
    },
    .b = {
      {},
      {},
      {.port = GPIO_PORT_D, .pin = 14},   /* B2 */
      {.port = GPIO_PORT_D, .pin = 15},   /* B3 */
      {.port = GPIO_PORT_D, .pin = 0},    /* B4 */
      {.port = GPIO_PORT_D, .pin = 1},    /* B5 */
      {.port = GPIO_PORT_B, .pin = 0},    /* B6 */
      {.port = GPIO_PORT_E, .pin = 4},    /* B7 */
    },
    .num_red_bits = 8,
    .num_green_bits = 8,
    .num_blue_bits = 8
  };
  settings.width = WIDTH;
  settings.height = HEIGHT;
  settings.framebuffer = framebuffer;
  settings.clut = clut_get_table();
  settings.clut_size = NUM_COLOR_INDICES;
  settings.timing = timing_config;
  settings.gpio_config = gpio_config;


  gpio_init_pin(&s_display_ctrl, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&s_display_current_ctrl, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

  StatusCode status;

  status = ltdc_init(&settings);
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("ltdc initialized\r\n");
  } else {
    LOG_DEBUG("ltdc cannot be initialized\r\n");
  }

  Framebuffer framebuffer_cfg = {0}; 
  status = framebuffer_init(&framebuffer_cfg, WIDTH, HEIGHT, &framebuffer);
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("framebuffer initialized\r\n");
  } else {
    LOG_DEBUG("framebuffer cannot be initialized\r\n");
  }

  GuiSettings gui_cfg = {.framebuffer = &framebuffer_cfg, .ltdc = &settings}; 
  status = gui_init(&gui_cfg);
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("Gui initialized\r\n");
  } else {
    LOG_DEBUG("Gui cannot be initialized\r\n");
  }

  Framebuffer framebuffer_cfg = {0}; 
  status = framebuffer_init(&framebuffer_cfg, WIDTH, HEIGHT, &framebuffer);
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("framebuffer initialized\r\n");
  } else {
    LOG_DEBUG("framebuffer cannot be initialized\r\n");
  }

  while (true) {

    printf("Testing framebuffer_clear to white"); 
    status = framebuffer_clear(&framebuffer_cfg, COLOR_INDEX_WHITE); 

    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("framebuffer_clear to white failed \r\n");
    } else {
      printf("framebuffer_clear succeeded \n");
    }

    printf("Testing framebuffer_get_pixels of current framebuffer\n"); 

    uint16_t i, j; 

    for (i = 0; i < WIDTH; i++) {
      for (i = 0; i < HEIGHT; i++) {
        status = framebuffer_get_pixel(&framebuffer_cfg,  i, j); 

        if (status != STATUS_CODE_OK){
          LOG_DEBUG("Getting frame pixel at %d, %d failed \n", i, j);
        } else {
          printf("Getting frame pixel at %d, %d, suceeded \n", i, j);
        }
      }
    }

    printf("Testing framebuffer_get_pixels of current framebuffer\n"); 

    uint16_t i, j; 

    for (i = 0; i < WIDTH; i++) {
      for (i = 0; i < HEIGHT; i++) {
        status = framebuffer_get_pixel(&framebuffer_cfg,  i, j); 

        if (status != STATUS_CODE_OK){
          LOG_DEBUG("Getting frame pixel at %d, %d failed \n", i, j);
        } else {
          printf("Getting frame pixel at %d, %d, suceeded \n", i, j);
        }
      }
    }

    printf("Testing gui_render");
    
    status = gui_render(); 

    if (status != STATUS_CODE_OK){
          LOG_DEBUG("Gui render failed");
        } else {
          printf("Gui render succeeded");
    }

    printf("Testing gui_drawpixel with drawing checkboard");
    //! draw pixel is literally an api call to ltdc_set_pixel anyways 

    status = draw_checkboard(COLOR_INDEX_BLACK, COLOR_INDEX_WHITE, 16);
    if (status != STATUS_CODE_OK){
          LOG_DEBUG("drawing checkboard failed");
        } else {
          printf("drawing checkboard failed");
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