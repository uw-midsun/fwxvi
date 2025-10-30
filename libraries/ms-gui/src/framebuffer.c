/************************************************************************************************
 * @file   framebuffer.c
 *
 * @brief  Source file for the framebuffer
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "framebuffer.h"

StatusCode framebuffer_init(Framebuffer *fb, uint16_t width, uint16_t height, uint8_t *buffer) {
  // TODO: Validate parameters (check for NULL pointers, zero dimensions)
  // TODO: Initialize fb->width, fb->height, fb->data
  // TODO: Return STATUS_CODE_OK on success, STATUS_CODE_INVALID_ARGS on invalid arguments
  return STATUS_CODE_OK;
}

StatusCode framebuffer_set_pixel(Framebuffer *fb, uint16_t x, uint16_t y, ColorIndex color_index) {
  // TODO: Calculate linear index from (x, y) coordinates
  // TODO: Write color_index to fb->data[index]
  // index = (y * fb->width) + x
  return STATUS_CODE_OK;
}

ColorIndex framebuffer_get_pixel(Framebuffer *fb, uint16_t x, uint16_t y) {
  // TODO: Calculate linear index from (x, y) coordinates
  // TODO: Return fb->data[index]
  // index = (y * fb->width) + x
  return 0;
}

StatusCode framebuffer_clear(Framebuffer *fb, ColorIndex color_index) {
  // TODO: Fill entire framebuffer with color_index
  // Size = fb->width * fb->height
  return STATUS_CODE_OK;
}

uint8_t *framebuffer_data(Framebuffer *fb) {
  // TODO: Return fb->data pointer
  return 0;
}
