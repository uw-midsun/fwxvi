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

/**
 * @defgroup arena
 * @brief    arena Firmware
 * @{
 */

typedef struct {
  uint8_t *base;
  uint8_t *end;
  uint8_t *current;
} arena;

void arena_init(arena *a, void *buffer, ptrdiff_t cap);
void arena_free(arena *a);
void arena_reset(arena *a);
void *arena_alloc(arena *a, ptrdiff_t size, ptrdiff_t align, ptrdiff_t count);

/** @} */
