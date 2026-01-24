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


void draw_grid(void) {
  // 1) Border frame
  gui_draw_line(0, 0, WIDTH - 1, 0, COLOR_INDEX_BLACK);
  gui_draw_line(0, 0, 0, HEIGHT - 1, COLOR_INDEX_BLACK);
  gui_draw_line(WIDTH - 1, 0, WIDTH - 1, HEIGHT - 1, COLOR_INDEX_BLACK);
  gui_draw_line(0, HEIGHT - 1, WIDTH - 1, HEIGHT - 1, COLOR_INDEX_BLACK);

  // 2) Diagonals (easy visual correctness check)
  gui_draw_line(0, 0, WIDTH - 1, HEIGHT - 1, COLOR_INDEX_BLACK);
  gui_draw_line(0, HEIGHT - 1, WIDTH - 1, 0, COLOR_INDEX_BLACK);

  // 3) Grid lines
  const uint16_t step = 8; // try 6, 8, 10, 16 depending on resolution

  for (uint16_t x = 0; x < WIDTH; x += step) {
    gui_draw_line(x, 0, x, HEIGHT - 1, COLOR_INDEX_BLACK);
  }
  for (uint16_t y = 0; y < HEIGHT; y += step) {
    gui_draw_line(0, y, WIDTH - 1, y, COLOR_INDEX_BLACK);
  }

  // 4) A little “X” box in the center (extra sanity check)
  uint16_t cx0 = WIDTH / 4,  cy0 = HEIGHT / 4;
  uint16_t cx1 = (3 * WIDTH) / 4, cy1 = (3 * HEIGHT) / 4;

  gui_draw_line(cx0, cy0, cx1, cy0, COLOR_INDEX_BLACK);
  gui_draw_line(cx1, cy0, cx1, cy1, COLOR_INDEX_BLACK);
  gui_draw_line(cx1, cy1, cx0, cy1, COLOR_INDEX_BLACK);
  gui_draw_line(cx0, cy1, cx0, cy0, COLOR_INDEX_BLACK);
  gui_draw_line(cx0, cy0, cx1, cy1, COLOR_INDEX_BLACK);
  gui_draw_line(cx0, cy1, cx1, cy0, COLOR_INDEX_BLACK);
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

    printf("testing if gui_fill_rect works");

    gui_fill_rect(50, 50, 100, 100, COLOR_INDEX_BLUE );
    delay_ms(1000);

    printf("testing if gui_render works"); 
    StatusCode status; 

    if (gui_render() != STATUS_CODE_OK){
      LOG_DEBUG("gui render failed");
    } else {
      printf("gui_render suceeded");
    }

    printf("testing if percentage bar works"); 

    int i; 
    for (i = 0; i < 100; i++){
      gui_progress_bar(50, 50, 200, 30, i, COLOR_INDEX_WHITE, COLOR_INDEX_BLUE);
      delay_ms(10); 
    }

    printf("testing if gui_draw_line works"); 
    draw_grid(); 
    delay_ms(1000);

    printf("Testing whether gui_display_text with hello world works"); 

    const char *text = "hello world";
    gui_display_text(50, 50, &text, COLOR_INDEX_BLACK); 

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