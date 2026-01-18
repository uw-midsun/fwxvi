/************************************************************************************************
 * @file   framebuffer.c
 *
 * @brief  Source file for the framebuffer
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "framebuffer.h"

StatusCode framebuffer_init(Framebuffer *fb, uint16_t width, uint16_t height, uint8_t *buffer) {
  if (fb == NULL || buffer == NULL || width == 0 || height == 0) {
    return STATUS_CODE_INVALID_ARGS;
  }

  fb->width = width;
  fb->height = height;
  fb->data = buffer;

  return STATUS_CODE_OK;
}

StatusCode framebuffer_set_pixel(Framebuffer *fb, uint16_t x, uint16_t y, ColorIndex color_index) {
  uint32_t index = (y * fb->width) + x;
  fb->data[index] = color_index;
  return STATUS_CODE_OK;
}

ColorIndex framebuffer_get_pixel(Framebuffer *fb, uint16_t x, uint16_t y) {
  uint32_t index = (y * fb->width) + x;
  return (fb->data[index]);
}

StatusCode framebuffer_clear(Framebuffer *fb, ColorIndex color_index) {
  uint32_t size = fb->width * fb->height;
  for (uint32_t i = 0; i < size; ++i) {
    fb->data[i] = color_index;
  }
  return STATUS_CODE_OK;
}

uint8_t *framebuffer_data(Framebuffer *fb) {
  return fb->data;
}
