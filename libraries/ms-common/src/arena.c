/************************************************************************************************
 * @file   arena.c
 *
 * @brief  Source code for the arena library
 *
 * @date   2026-01-10
 * @author -
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "arena.h"

StatusCode arena_init(Arena *a, void *buffer, ptrdiff_t cap) {
  if (a == NULL || buffer == NULL || cap <= 0) {
    return STATUS_CODE_INVALID_ARGS;
  }

  a->base = (uint8_t *)buffer;
  a->end = a->base + cap;
  a->current = a->end;
  return STATUS_CODE_OK;
}

StatusCode arena_reset(Arena *a) {
  if (a == NULL || a->base == NULL || a->end == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  a->current = a->end;
  return STATUS_CODE_OK;
}

void *arena_alloc(Arena *a, ptrdiff_t size, ptrdiff_t align, ptrdiff_t count) {
  void *out = NULL;

  if (a == NULL || a->base == NULL || a->end == NULL || a->current == NULL) {
    return NULL;
  }

  if (size <= 0 || count <= 0 || align <= 0) {
    return NULL;
  }

  // align must be a power of two
  if ((align & (align - 1)) != 0) {
    return NULL;
  }

  // overflow check
  if (count != 0 && size > PTRDIFF_MAX / count) {
    return NULL;
  }

  ptrdiff_t total = size * count;

  uintptr_t start = (uintptr_t)a->current - (uintptr_t)total;

  // logic to align down if needed
  uintptr_t mask = (uintptr_t)(align - 1);
  start &= ~mask;

  uint8_t *start_ptr = (uint8_t *)start;
  if (start_ptr < a->base) {
    return NULL;
  }

  a->current = start_ptr;
  out = memset(start_ptr, 0, (size_t)total);

  return out;
}
