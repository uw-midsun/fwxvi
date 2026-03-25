/************************************************************************************************
 * @file    test_sdl_dummy.c
 *
 * @brief   Dummy SDL2 test to verify SDL2 integration in x86 builds
 *
 * @date    2025-09-07
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <SDL2/SDL.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Inter-component Headers */
#include "unity.h"

/* Intra-component Headers */

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static void get_test_results_paths(const char *filename, char *out_dir, size_t dir_size, char *out_file, size_t file_size) {
  char file_path[512];
  strncpy(file_path, __FILE__, sizeof(file_path) - 1);
  file_path[sizeof(file_path) - 1] = '\0';

  char *dir = dirname(file_path);
  snprintf(out_dir, dir_size, "%s/test_results", dir);
  snprintf(out_file, file_size, "%s/%s", out_dir, filename);
}

void setup_test(void) {
  TEST_ASSERT_EQUAL_MESSAGE(0, SDL_Init(SDL_INIT_VIDEO), "SDL_Init failed");

  window = SDL_CreateWindow("SDL Dummy Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 200, 200, SDL_WINDOW_SHOWN);
  TEST_ASSERT_NOT_NULL_MESSAGE(window, "SDL_CreateWindow failed");

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  TEST_ASSERT_NOT_NULL_MESSAGE(renderer, "SDL_CreateRenderer failed");
}

void teardown_test(void) {
  if (renderer != NULL) {
    SDL_DestroyRenderer(renderer);
    renderer = NULL;
  }
  if (window != NULL) {
    SDL_DestroyWindow(window);
    window = NULL;
  }
  SDL_Quit();
}

void test_sdl_renders_rectangle(void) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_Rect rect = { 50, 50, 100, 100 };
  TEST_ASSERT_EQUAL_MESSAGE(0, SDL_RenderFillRect(renderer, &rect), "SDL_RenderFillRect failed");

  SDL_RenderPresent(renderer);
  SDL_Delay(50);

  TEST_PASS_MESSAGE("Rendered rectangle successfully");
}

void test_sdl_saves_output_to_file(void) {
  TEST_ASSERT_NOT_NULL(renderer);

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
  SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
  SDL_Rect rect = { 30, 30, 140, 140 };
  SDL_RenderFillRect(renderer, &rect);
  SDL_RenderPresent(renderer);

  char out_dir[512];
  char out_file[512];
  get_test_results_paths("test_dummy_sdl2.bmp", out_dir, sizeof(out_dir), out_file, sizeof(out_file));

  struct stat st = { 0 };
  if (stat(out_dir, &st) == -1) {
    mkdir(out_dir, 0755);
  }

  SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, 200, 200, 32, SDL_PIXELFORMAT_RGBA32);
  TEST_ASSERT_NOT_NULL_MESSAGE(surface, "SDL_CreateRGBSurfaceWithFormat failed");

  TEST_ASSERT_EQUAL_MESSAGE(0, SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA32, surface->pixels, surface->pitch), "SDL_RenderReadPixels failed");

  TEST_ASSERT_EQUAL_MESSAGE(0, SDL_SaveBMP(surface, out_file), "SDL_SaveBMP failed");

  SDL_FreeSurface(surface);

  TEST_PASS_MESSAGE("Saved output.bmp to test_results/");
}
