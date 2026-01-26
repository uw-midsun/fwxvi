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
#include "clut.h"
#include "delay.h"
#include "framebuffer.h"
#include "gpio.h"
#include "gui.h"
#include "log.h"
#include "ltdc.h"
#include "mcu.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */

#ifdef STM32L4P5xx         /* Framebuffer takes up too much RAM on other STMs otherwise*/
#define DISPLAY_WIDTH 480  /**< Width of the display */
#define DISPLAY_HEIGHT 272 /**< Height of the display */
#else
#define DISPLAY_WIDTH 1  /**< Width of the display */
#define DISPLAY_HEIGHT 1 /**< Height of the display */
#endif

static uint8_t framebuffer[DISPLAY_WIDTH * DISPLAY_HEIGHT] __attribute__((aligned(32)));
static GpioAddress s_display_ctrl = { .port = GPIO_PORT_A, .pin = 0 };
static GpioAddress s_display_current_ctrl = { .port = GPIO_PORT_A, .pin = 1 };
static LtdcSettings settings = { 0 };

static void draw_grid(void) {
  // 1) Border frame
  gui_draw_line(0, 0, DISPLAY_WIDTH - 1, 0, COLOR_INDEX_BLACK);
  gui_draw_line(0, 0, 0, DISPLAY_HEIGHT - 1, COLOR_INDEX_BLACK);
  gui_draw_line(DISPLAY_WIDTH - 1, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1, COLOR_INDEX_BLACK);
  gui_draw_line(0, DISPLAY_HEIGHT - 1, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1, COLOR_INDEX_BLACK);

  // 2) Diagonals (easy visual correctness check)
  gui_draw_line(0, 0, DISPLAY_WIDTH - 1, DISPLAY_WIDTH - 1, COLOR_INDEX_BLACK);
  gui_draw_line(0, DISPLAY_HEIGHT - 1, DISPLAY_WIDTH - 1, 0, COLOR_INDEX_BLACK);

  // 3) Grid lines
  const uint16_t step = 8;  // try 6, 8, 10, 16 depending on resolution

  for (uint16_t x = 0; x < DISPLAY_WIDTH; x += step) {
    gui_draw_line(x, 0, x, DISPLAY_HEIGHT - 1, COLOR_INDEX_BLACK);
  }
  for (uint16_t y = 0; y < DISPLAY_HEIGHT; y += step) {
    gui_draw_line(0, y, DISPLAY_WIDTH - 1, y, COLOR_INDEX_BLACK);
  }

  // 4) A little “X” box in the center (extra sanity check)
  const uint16_t cx0 = DISPLAY_WIDTH / 4;
  const uint16_t cy0 = DISPLAY_HEIGHT / 4;
  const uint16_t cx1 = (3 * DISPLAY_WIDTH) / 4;
  const uint16_t cy1 = (3 * DISPLAY_HEIGHT) / 4;

  gui_draw_line(cx0, cy0, cx1, cy0, COLOR_INDEX_BLACK);
  gui_draw_line(cx1, cy0, cx1, cy1, COLOR_INDEX_BLACK);
  gui_draw_line(cx1, cy1, cx0, cy1, COLOR_INDEX_BLACK);
  gui_draw_line(cx0, cy1, cx0, cy0, COLOR_INDEX_BLACK);
  gui_draw_line(cx0, cy0, cx1, cy1, COLOR_INDEX_BLACK);
  gui_draw_line(cx0, cy1, cx1, cy0, COLOR_INDEX_BLACK);
}

TASK(framebuffer_api, TASK_STACK_1024) {
  const LtdcTimingConfig timing_config = { .hsync = 4, .vsync = 4, .hbp = 43, .vbp = 12, .hfp = 8, .vfp = 8 };

  const LtdcGpioConfig gpio_config = {
    .clk = { .port = GPIO_PORT_A, .pin = 4 },
    .hsync = { .port = GPIO_PORT_C, .pin = 2 },
    .vsync = { .port = GPIO_PORT_B, .pin = 11 },
    .de = { .port = GPIO_PORT_C, .pin = 0 },
    .r = {
      {},
      {},
      { .port = GPIO_PORT_E, .pin = 15 }, /* R2 */
      { .port = GPIO_PORT_D, .pin = 8 },  /* R3 */
      { .port = GPIO_PORT_D, .pin = 9 },  /* R4 */
      { .port = GPIO_PORT_D, .pin = 10 }, /* R5 */
      { .port = GPIO_PORT_D, .pin = 11 }, /* R6 */
      { .port = GPIO_PORT_D, .pin = 12 }, /* R7 */
    },
    .g = {
      {},
      {},
      { .port = GPIO_PORT_E, .pin = 9 },  /* G2 */
      { .port = GPIO_PORT_E, .pin = 10 }, /* G3 */
      { .port = GPIO_PORT_E, .pin = 11 }, /* G4 */
      { .port = GPIO_PORT_E, .pin = 12 }, /* G5 */
      { .port = GPIO_PORT_E, .pin = 13 }, /* G6 */
      { .port = GPIO_PORT_E, .pin = 14 }, /* G7 */
    },
    .b = {
      {},
      {},
      { .port = GPIO_PORT_D, .pin = 14 }, /* B2 */
      { .port = GPIO_PORT_D, .pin = 15 }, /* B3 */
      { .port = GPIO_PORT_D, .pin = 0 },  /* B4 */
      { .port = GPIO_PORT_D, .pin = 1 },  /* B5 */
      { .port = GPIO_PORT_B, .pin = 0 },  /* B6 */
      { .port = GPIO_PORT_E, .pin = 4 },  /* B7 */
    },
    .num_red_bits = 8,
    .num_green_bits = 8,
    .num_blue_bits = 8,
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

  StatusCode status = STATUS_CODE_OK;

  status = ltdc_init(&settings);
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("ltdc initialized\r\n");
  } else {
    LOG_DEBUG("ltdc cannot be initialized: %d\r\n", status);
  }

  Framebuffer framebuffer_cfg = { 0 };
  status = framebuffer_init(&framebuffer_cfg, DISPLAY_WIDTH, DISPLAY_HEIGHT, framebuffer);
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("framebuffer initialized\r\n");
  } else {
    LOG_DEBUG("framebuffer cannot be initialized: %d\r\n", status);
  }

  GuiSettings gui_cfg = {
    .framebuffer = framebuffer_cfg,
    .ltdc = settings,
  };

  status = gui_init(&gui_cfg);
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("Gui initialized\r\n");
  } else {
    LOG_DEBUG("Gui cannot be initialized: %d\r\n", status);
  }

  while (true) {
    LOG_DEBUG("testing if gui_fill_rect works\r\n");
    status = gui_fill_rect(50, 50, 100, 100, COLOR_INDEX_BLUE);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("gui_fill_rect failed: %d\r\n", status);
    }
    delay_ms(1000);

    LOG_DEBUG("testing if gui_render works\r\n");
    status = gui_render();
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("gui_render failed: %d\r\n", status);
    } else {
      LOG_DEBUG("gui_render succeeded\r\n");
    }

    LOG_DEBUG("testing if percentage bar works\r\n");
    for (uint16_t i = 0; i <= 100; i++) {
      status = gui_progress_bar(50, 50, 200, 30, i, COLOR_INDEX_WHITE, COLOR_INDEX_BLUE);
      if (status != STATUS_CODE_OK) {
        LOG_DEBUG("gui_progress_bar failed: %d\r\n", status);
        break;
      }
      delay_ms(10);
    }

    LOG_DEBUG("testing if gui_draw_line works\r\n");
    draw_grid();
    status = gui_render();
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("gui_render after draw_grid failed: %d\r\n", status);
    }
    delay_ms(1000);

    LOG_DEBUG("Testing whether gui_display_text with hello world works\r\n");
    status = gui_display_text(50, 50, "hello world", COLOR_INDEX_BLACK);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("gui_display_text failed: %d\r\n", status);
    }

    status = gui_render();
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("gui_render after gui_display_text failed: %d\r\n", status);
    }

    delay_ms(1000);
  }
}

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
int main(int argc, char *argv[]) {
  mpxe_init(argc, argv);
#else
int main(void) {
#endif
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(framebuffer_api, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?\r\n");
  return 0;
}
