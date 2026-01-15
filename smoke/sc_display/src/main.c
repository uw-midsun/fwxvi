/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for sc_display (Cycles through different checkerboard colors)
 *
 * @date   2026-01-12
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
#include "ltdc.h"
#include "clut.h"
/* Intra-component Headers */

#define DISPLAY_WIDTH  480
#define DISPLAY_HEIGHT 272

static uint8_t framebuffer[DISPLAY_WIDTH * DISPLAY_HEIGHT] __attribute__((aligned(32)));
static GpioAddress s_display_ctrl =  { .port = GPIO_PORT_A, .pin = 0 };
static GpioAddress s_display_current_ctrl = { .port = GPIO_PORT_A, .pin = 1 }; 
static LtdcSettings settings = { 0 };

StatusCode ltdc_display_init() {
  /* From: https://www.buydisplay.com/download/ic/ST7282.pdf */
  //TODO move values to macros 
  LtdcTimingConfig timing_config = { 
    .hsync = 4, 
    .vsync = 4, 
    .hbp = 43, 
    .vbp = 12, 
    .hfp = 8, 
    .vfp = 8 
  };

  LtdcGpioConfig gpio_config = { 
    .clk = {.port = GPIO_PORT_A, .pin = 4},
    .hsync = {.port = GPIO_PORT_C, .pin = 2},
    .vsync = {.port = GPIO_PORT_B, .pin = 11},
    .de = {.port = GPIO_PORT_C, .pin = 0},
    .r = {
      {},
      {},
      {.port = GPIO_PORT_E, .pin = 15},  /* R2 */ 
      {.port = GPIO_PORT_D, .pin = 8},  /* R3 */ 
      {.port = GPIO_PORT_D, .pin = 9},  /* R4 */ 
      {.port = GPIO_PORT_D, .pin = 10},  /* R5 */ 
      {.port = GPIO_PORT_D, .pin = 11},  /* R6 */ 
      {.port = GPIO_PORT_D, .pin = 12}, /* R7 */ 
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
      {.port = GPIO_PORT_D, .pin = 14},  /* B2 */ 
      {.port = GPIO_PORT_D, .pin = 15},  /* B3 */ 
      {.port = GPIO_PORT_D, .pin = 0},  /* B4 */ 
      {.port = GPIO_PORT_D, .pin = 1},  /* B5 */ 
      {.port = GPIO_PORT_B, .pin = 0}, /* B6 */ 
      {.port = GPIO_PORT_E, .pin = 4}, /* B7 */ 
    },
    .num_red_bits = 8,
    .num_green_bits = 8,
    .num_blue_bits = 8
  }; 
  settings.width = DISPLAY_WIDTH; 
  settings.height = DISPLAY_HEIGHT; 
  settings.framebuffer = framebuffer; 
  settings.clut = clut_get_table(); 
  settings.clut_size = NUM_COLOR_INDICES; 
  settings.timing = timing_config; 
  settings.gpio_config = gpio_config; 
  
  gpio_init_pin(&s_display_ctrl, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&s_display_current_ctrl, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  return ltdc_init(&settings);
}

StatusCode draw_checkerboard(ColorIndex color1, ColorIndex color2, uint16_t square_size) {
  /* Direct framebuffer access is much faster than calling ltdc_set_pixel 130k times */
  for (uint16_t y = 0; y < DISPLAY_HEIGHT; y++) {
    for (uint16_t x = 0; x < DISPLAY_WIDTH; x++) {
      uint16_t square_x = x / square_size;
      uint16_t square_y = y / square_size;
      ColorIndex color = ((square_x + square_y) % 2 == 0) ? color1 : color2;
      status_ok_or_return(ltdc_set_pixel(x, y, color));
    }
  }
  /* Apply changes */
  return ltdc_draw();
}

TASK(sc_display, TASK_STACK_1024) {
  StatusCode status = ltdc_display_init();
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("LTDC init failed: %d", status);
    delay_ms(1000U); 
    return;
  }
  status = draw_checkerboard(COLOR_INDEX_YELLOW, COLOR_INDEX_BLUE, 16);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Draw failed: %d", status);
      delay_ms(1000U); 
  }

  while (true) {
    delay_ms(1000); 
    LOG_DEBUG("I'm alive");
    status = draw_checkerboard(COLOR_INDEX_BLACK, COLOR_INDEX_WHITE, 16);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Draw failed: %d", status);
      delay_ms(1000U); 
  }
  status = draw_checkerboard(COLOR_INDEX_YELLOW, COLOR_INDEX_BLUE, 16);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Draw failed: %d", status);
      delay_ms(1000U); 
  }
  status = draw_checkerboard(COLOR_INDEX_WHITE, COLOR_INDEX_RED, 16);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Draw failed: %d", status);
      delay_ms(1000U); 
  }
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

  tasks_init_task(sc_display, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}