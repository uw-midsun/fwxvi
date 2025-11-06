/************************************************************************************************
 * @file    test_ltdc_sim.c
 *
 * @brief   LTDC SDL2 simulation driver test
 *
 * @date    2025-10-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <SDL2/SDL.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Inter-component Headers */
#include "unity.h"

/* Intra-component Headers */
#include "clut.h"
#include "ltdc.h"

#define TEST_WIDTH 64
#define TEST_HEIGHT 64

static uint8_t framebuffer[TEST_WIDTH * TEST_HEIGHT];

static void clear_framebuffer(void) {
  memset(framebuffer, 0, sizeof(framebuffer));
}

void setup_test(void) {
  clear_framebuffer();
}

void teardown_test(void) {}

void test_ltdc_initialization(void) {
  LtdcSettings cfg = { .width = TEST_WIDTH, .height = TEST_HEIGHT, .framebuffer = framebuffer, .clut = (ClutEntry *)clut_get_table(), .clut_size = NUM_COLOR_INDICES };

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ltdc_init(&cfg));

  /* Renderer should exist */
  SDL_Renderer *renderer = (SDL_Renderer *)ltdc_get_renderer();
  TEST_ASSERT_NOT_NULL(renderer);

  /* Save blank frame */
  save_ltdc_frame("libraries/gui/test/test_results/init_frame.bmp");
}

void test_ltdc_draw_red_square(void) {
  /* Draw red square */
  for (uint16_t y = 16; y < 48; y++) {
    for (uint16_t x = 16; x < 48; x++) {
      ltdc_set_pixel(x, y, COLOR_INDEX_RED);
    }
  }

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ltdc_draw());

  /* Save frame */
  save_ltdc_frame("libraries/gui/test/test_results/red_square.bmp");
}

void test_ltdc_draw_multiple_colors(void) {
  /* Draw a gradient */
  for (uint16_t y = 0; y < TEST_HEIGHT; y++) {
    for (uint16_t x = 0; x < TEST_WIDTH; x++) {
      if (x < TEST_WIDTH / 2) {
        ltdc_set_pixel(x, y, COLOR_INDEX_BLUE);
      } else {
        ltdc_set_pixel(x, y, COLOR_INDEX_GREEN);
      }
    }
  }

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ltdc_draw());

  /* Save frame */
  save_ltdc_frame("libraries/gui/test/test_results/blue_green_gradient.bmp");
}

void test_ltdc_full_frame(void) {
  /* Fill entire framebuffer with yellow */
  for (uint16_t y = 0; y < TEST_HEIGHT; y++) {
    for (uint16_t x = 0; x < TEST_WIDTH; x++) {
      ltdc_set_pixel(x, y, COLOR_INDEX_YELLOW);
    }
  }

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ltdc_draw());
  save_ltdc_frame("libraries/gui/test/test_results/full_yellow.bmp");
}
