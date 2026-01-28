#pragma once

/************************************************************************************************
 * @file   arena.h
 *
 * @brief  Header file for the arena library
 *
 * @date   2026-01-10
 * @author -
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "status.h"

/**
 * @defgroup arena
 * @brief    arena Firmware
 * @{
 */

/** @brief  Linear arena allocator state */
typedef struct {
  uint8_t *base;    /**< Start of the arena buffer */
  uint8_t *end;     /**< One-past-end of the arena buffer */
  uint8_t *current; /**< Current allocation pointer */
} Arena;

/**
 * @brief   Initialize an arena to use a provided buffer
 * @param   a      Arena instance to initialize
 * @param   buffer Backing buffer for allocations
 * @param   cap    Size of the buffer in bytes
 * @return  STATUS_CODE_OK on success, or an error code on failure
 */
StatusCode arena_init(Arena *a, void *buffer, ptrdiff_t cap);


/**
 * @brief   Reset the arena to reuse all capacity
 * @param   a  Arena instance to reset
 * @return  STATUS_CODE_OK on success, or an error code on failure
 */
StatusCode arena_reset(Arena *a);

/**
 * @brief   Allocate memory from the arena
 * @param   a     Arena instance to allocate from
 * @param   size  Size of each element in bytes
 * @param   align Alignment in bytes (power of two)
 * @param   count Number of elements to allocate
 * @param   out   Output pointer for the allocated memory
 * @return  STATUS_CODE_OK on success, or an error code on failure
 */
StatusCode arena_alloc(Arena *a, ptrdiff_t size, ptrdiff_t align, ptrdiff_t count, void **out);

/** @} */
