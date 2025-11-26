/************************************************************************************************
 * @file   ltdc.c
 *
 * @brief  Source file for the LTDC driver on x86
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <SDL2/SDL.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Inter-component Headers */
#include "log.h"

/* Intra-component Headers */
#include "clut.h"
#include "ltdc.h"

typedef struct {
  uint16_t width;
  uint16_t height;
  uint8_t *framebuffer;
  ClutEntry *clut;
  uint16_t clut_size;

  SDL_Window *window;
  SDL_Renderer *renderer;
} LtdcSimSettings;

static LtdcSimSettings s_ltdc_sim_settings = { 0 };

StatusCode ltdc_init(LtdcSettings *settings) {
  if (!settings || !settings->framebuffer || !settings->clut) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_ltdc_sim_settings.width = settings->width;
  s_ltdc_sim_settings.height = settings->height;
  s_ltdc_sim_settings.framebuffer = settings->framebuffer;
  s_ltdc_sim_settings.clut = settings->clut;
  s_ltdc_sim_settings.clut_size = settings->clut_size;

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    LOG_DEBUG("SDL_Init Error: %s\n", SDL_GetError());
    return STATUS_CODE_INTERNAL_ERROR;
  }

  s_ltdc_sim_settings.window = SDL_CreateWindow("GUI Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, s_ltdc_sim_settings.width, s_ltdc_sim_settings.height, SDL_WINDOW_SHOWN);
  if (!s_ltdc_sim_settings.window) {
    LOG_DEBUG("SDL_CreateWindow Error: %s\n", SDL_GetError());
    SDL_Quit();
    return STATUS_CODE_INTERNAL_ERROR;
  }

  s_ltdc_sim_settings.renderer = SDL_CreateRenderer(s_ltdc_sim_settings.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!s_ltdc_sim_settings.renderer) {
    LOG_DEBUG("SDL_CreateRenderer Error: %s\n", SDL_GetError());
    SDL_DestroyWindow(s_ltdc_sim_settings.window);
    SDL_Quit();
    return STATUS_CODE_INTERNAL_ERROR;
  }

  SDL_SetRenderDrawBlendMode(s_ltdc_sim_settings.renderer, SDL_BLENDMODE_NONE);
  return STATUS_CODE_OK;
}

StatusCode ltdc_draw(void) {
  if (!s_ltdc_sim_settings.framebuffer || !s_ltdc_sim_settings.clut) {
    SDL_SetRenderDrawColor(s_ltdc_sim_settings.renderer, 100, 100, 100, 255);
    SDL_RenderClear(s_ltdc_sim_settings.renderer);
    SDL_RenderPresent(s_ltdc_sim_settings.renderer);
    return STATUS_CODE_INVALID_ARGS;
  }

  SDL_SetRenderDrawColor(s_ltdc_sim_settings.renderer, 0, 0, 0, 255);
  SDL_RenderClear(s_ltdc_sim_settings.renderer);

  for (uint16_t y = 0; y < s_ltdc_sim_settings.height; y++) {
    for (uint16_t x = 0; x < s_ltdc_sim_settings.width; x++) {
      size_t idx = y * s_ltdc_sim_settings.width + x;
      uint8_t color_idx = s_ltdc_sim_settings.framebuffer[idx];
      if (color_idx >= s_ltdc_sim_settings.clut_size) continue;

      ClutEntry entry = s_ltdc_sim_settings.clut[color_idx];
      SDL_SetRenderDrawColor(s_ltdc_sim_settings.renderer, entry.red, entry.green, entry.blue, 255);
      SDL_RenderDrawPoint(s_ltdc_sim_settings.renderer, x, y);
    }
  }

  SDL_RenderPresent(s_ltdc_sim_settings.renderer);
  return STATUS_CODE_OK;
}

StatusCode ltdc_set_pixel(uint16_t x, uint16_t y, ColorIndex color_index) {
  if (!s_ltdc_sim_settings.framebuffer) return STATUS_CODE_INVALID_ARGS;
  if (x >= s_ltdc_sim_settings.width || y >= s_ltdc_sim_settings.height) return STATUS_CODE_INVALID_ARGS;

  s_ltdc_sim_settings.framebuffer[y * s_ltdc_sim_settings.width + x] = (uint8_t)color_index;
  return STATUS_CODE_OK;
}

void *ltdc_get_renderer(void) {
  return (void *)(s_ltdc_sim_settings.renderer);
}

void save_ltdc_frame(const char *filename) {
  if (!filename) return;

  /* Ensure the directory exists */
  char dir[512];
  strncpy(dir, filename, sizeof(dir) - 1);
  dir[sizeof(dir) - 1] = '\0';

  char *dir_name = dirname(dir);
  struct stat st = { 0 };
  if (stat(dir_name, &st) == -1) {
    if (mkdir(dir_name, 0755) != 0) {
      LOG_DEBUG("Failed to create directory %s: %s\r\n", dir_name, strerror(errno));
    }
  }

  /* Create surface and read pixels */
  SDL_Renderer *renderer = ltdc_get_renderer();
  SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, s_ltdc_sim_settings.width, s_ltdc_sim_settings.height, 32, SDL_PIXELFORMAT_RGBA32);

  if (surface == NULL) {
    LOG_DEBUG("Failed to create SDL surface for saving frame\r\n");
    return;
  }

  if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA32, surface->pixels, surface->pitch) != 0) {
    LOG_DEBUG("SDL_RenderReadPixels failed: %s\r\n", SDL_GetError());
    SDL_FreeSurface(surface);
    return;
  }

  if (SDL_SaveBMP(surface, filename) != 0) {
    LOG_DEBUG("SDL_SaveBMP failed: %s\r\n", SDL_GetError());
  }

  SDL_FreeSurface(surface);
}

bool ltdc_process_events(void) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      return false;
    }
  }
  return true;
}

void ltdc_delay_frame(uint32_t fps) {
  static uint32_t last_time = 0;
  uint32_t current_time = SDL_GetTicks();
  uint32_t frame_time = 1000 / fps;

  if (current_time - last_time < frame_time) {
    SDL_Delay(frame_time - (current_time - last_time));
  }
  last_time = SDL_GetTicks();
}

void ltdc_cleanup(void) {
  if (s_ltdc_sim_settings.renderer) {
    SDL_DestroyRenderer(s_ltdc_sim_settings.renderer);
    s_ltdc_sim_settings.renderer = NULL;
  }

  if (s_ltdc_sim_settings.window) {
    SDL_DestroyWindow(s_ltdc_sim_settings.window);
    s_ltdc_sim_settings.window = NULL;
  }

  SDL_Quit();
}
