#pragma once

/************************************************************************************************
 * @file   framebuffer.h
 *
 * @brief  Header file for the framebuffer
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "clut.h"

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

/**
 * @brief   Framebuffer structure
 *          The framebuffer stores 8-bit indices into the CLUT
 */
typedef struct {
  uint16_t width;  /**< Width in pixels */
  uint16_t height; /**< Height in pixels */
  uint8_t *data;   /**< Pointer to framebuffer data (size = width * height) */
} Framebuffer;

/**
 * @brief   Initialize the framebuffer structure
 * @param   fb Pointer to framebuffer structure
 * @param   width Width in pixels
 * @param   height Height in pixels
 * @param   buffer Pointer to preallocated framebuffer data
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode framebuffer_init(Framebuffer *fb, uint16_t width, uint16_t height, uint8_t *buffer);

/**
 * @brief   Set a pixel value (CLUT index) in the framebuffer
 * @param   fb Pointer to framebuffer structure
 * @param   x X coordinate
 * @param   y Y coordinate
 * @param   color_index CLUT color index
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode framebuffer_set_pixel(Framebuffer *fb, uint16_t x, uint16_t y, ColorIndex color_index);

/**
 * @brief   Get a pixel value (CLUT index) from the framebuffer
 * @param   fb Pointer to framebuffer structure
 * @param   x X coordinate
 * @param   y Y coordinate
 * @return  color_index value
 */
ColorIndex framebuffer_get_pixel(Framebuffer *fb, uint16_t x, uint16_t y);

/**
 * @brief   Fill entire framebuffer with one color index
 * @param   fb Pointer to framebuffer structure
 * @param   color_index CLUT color index
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode framebuffer_clear(Framebuffer *fb, ColorIndex color_index);

/**
 * @brief   Get framebuffer raw pointer
 * @param   fb Pointer to framebuffer structure
 * @return  Pointer to the framebuffer data
 */
uint8_t *framebuffer_data(Framebuffer *fb);

/** @} */
