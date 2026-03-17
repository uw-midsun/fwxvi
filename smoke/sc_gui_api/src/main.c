/************************************************************************************************
 * @file    main.c
 *
 * @brief   Main
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
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
#include "display.h"
#include "steering_hw_defs.h"

/* Intra-component Headers */

#ifdef STM32L4P5xx         /* Framebuffer takes up too much RAM on other STMs otherwise*/
#define DISPLAY_WIDTH 480  /**< Width of the display */
#define DISPLAY_HEIGHT 272 /**< Height of the display */
#else
#define DISPLAY_WIDTH 1  /**< Width of the display */
#define DISPLAY_HEIGHT 1 /**< Height of the display */
#endif

#define NUMBER_OF_RED_BITS 8
#define NUMBER_OF_GREEN_BITS 8
#define NUMBER_OF_BLUE_BITS 8

static uint8_t framebuffer[DISPLAY_WIDTH * DISPLAY_HEIGHT] __attribute__((aligned(32)));
static GpioAddress s_display_ctrl = GPIO_STEERING_DISPLAY_CTRL;
static GpioAddress s_display_pwm = GPIO_STEERING_BACKLIGHT;
static LtdcSettings settings = { 0 };
static Framebuffer framebuffer_cfg = { 0 };

static GuiSettings gui_cfg = {
  .framebuffer = &framebuffer_cfg,
  .ltdc = &settings,
};

static void draw_grid(void) {
  // 1) Border frame
  gui_draw_line(0, 0, DISPLAY_WIDTH - 1, 0, COLOR_INDEX_WHITE);
  gui_draw_line(0, 0, 0, DISPLAY_HEIGHT - 1, COLOR_INDEX_WHITE);
  gui_draw_line(DISPLAY_WIDTH - 1, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1, COLOR_INDEX_WHITE);
  gui_draw_line(0, DISPLAY_HEIGHT - 1, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1, COLOR_INDEX_WHITE);

  // 3) Grid lines
  const uint16_t step = 8;  // try 6, 8, 10, 16 depending on resolution

  for (uint16_t x = 0; x < DISPLAY_WIDTH; x += step) {
    gui_draw_line(x, 0, x, DISPLAY_HEIGHT - 1, COLOR_INDEX_WHITE);
  }
  for (uint16_t y = 0; y < DISPLAY_HEIGHT; y += step) {
    gui_draw_line(0, y, DISPLAY_WIDTH - 1, y, COLOR_INDEX_WHITE);
  }
}

TASK(sc_gui_api, TASK_STACK_1024) {
  LtdcTimingConfig timing_config = {
    .hsync = HORIZONTAL_SYNC_WIDTH, .vsync = VERTICAL_SYNC_WIDTH, .hbp = HORIZONTAL_BACK_PORCH, .vbp = VERTICAL_BACK_PORCH, .hfp = HORIZONTAL_FRONT_PORCH, .vfp = VERTICAL_FRONT_PORCH
  };

  LtdcGpioConfig gpio_config = { .clk = GPIO_STEERING_DISPLAY_LTDC_CLOCK,
                                 .hsync = GPIO_STEERING_DISPLAY_LTDC_HSYNC,
                                 .vsync = GPIO_STEERING_DISPLAY_LTDC_VSYNC,
                                 .de = GPIO_STEERING_DISPLAY_LTDC_DE,
                                 .r = GPIO_STEERING_DISPLAY_LTDC_RED_PINS,
                                 .g = GPIO_STEERING_DISPLAY_LTDC_GREEN_PINS,
                                 .b = GPIO_STEERING_DISPLAY_LTDC_BLUE_PINS,
                                 .num_red_bits = NUMBER_OF_RED_BITS,
                                 .num_green_bits = NUMBER_OF_GREEN_BITS,
                                 .num_blue_bits = NUMBER_OF_BLUE_BITS };

  settings.width = DISPLAY_WIDTH;
  settings.height = DISPLAY_HEIGHT;
  settings.framebuffer = framebuffer;
  settings.clut = clut_get_table();
  settings.clut_size = NUM_COLOR_INDICES;
  settings.timing = timing_config;
  settings.gpio_config = gpio_config;

  gpio_init_pin(&s_display_ctrl, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&s_display_pwm, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

  StatusCode status = STATUS_CODE_OK;

  status = gui_init(&gui_cfg);
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("Gui initialized\r\n");
  } else {
    LOG_DEBUG("Gui cannot be initialized: %d\r\n", status);
  }

  while (true) {
    LOG_DEBUG("testing if gui_draw_line works\r\n");
    draw_grid();
    status = ltdc_draw();
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("ltdc_draw after draw_grid failed: %d\r\n", status);
    }
    delay_ms(3000);

    /* Clear grid lines */
    LOG_DEBUG("Clearing grid lines\r\n");
    status = gui_fill_rect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, COLOR_INDEX_BLACK);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("gui_fill_rect failed: %d\r\n", status);
      delay_ms(10);
    }

    status = ltdc_draw();
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("ltdc_draw after draw_grid failed: %d\r\n", status);
    }
    delay_ms(1000);

    LOG_DEBUG("testing if gui_fill_rect works\r\n");
    status = gui_fill_rect(50, 150, 100, 100, COLOR_INDEX_BLUE);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("gui_fill_rect failed: %d\r\n", status);
      delay_ms(10);
    }
    delay_ms(1000);

    LOG_DEBUG("testing if ltdc_draw works\r\n");
    status = ltdc_draw();
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("ltdc_draw failed: %d\r\n", status);
      delay_ms(10);
    } else {
      LOG_DEBUG("ltdc_draw succeeded\r\n");
      delay_ms(10);
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

  tasks_init_task(sc_gui_api, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?\r\n");
  return 0;
}
