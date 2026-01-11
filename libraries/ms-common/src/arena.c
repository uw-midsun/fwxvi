/************************************************************************************************
 * @file   arena.c
 *
 * @brief  Source code for the arena library
 *
 * @date   2026-01-10
 * @author -
 ************************************************************************************************/

/* Standard library Headers */
#include <stdlib.h>
#include <string.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "arena.h"

void arena_init(arena *a, void *buffer, ptrdiff_t cap) {
  a->base = (uint8_t *)buffer;
  if (cap <= 0) {
    abort();
  }
  if (a->base != NULL) {
    a->end = a->base + cap;
  } else {
    a->end = NULL;
  }
  a->current = a->end;
}

// note: if arena is dynamically allocated, the caller must free(buffer) before calling arena_free
void arena_free(arena *a) {
  a->base = NULL;
  a->end = NULL;
  a->current = NULL;
}

void arena_reset(arena *a) {
  a->current = a->end;
}

void *arena_alloc(arena *a, ptrdiff_t size, ptrdiff_t align, ptrdiff_t count) {
  if (!a || !a->base || !a->end || !a->current) {
    abort();
  }
  if (size <= 0 || count < 0 || align <= 0) {
    abort();
  }

  // align must be a power of two
  if (!((align & (align - 1)) == 0)) {
    abort();
  }

  // overflow check
  if (count != 0 && size > PTRDIFF_MAX / count) {
    abort();
  }

  ptrdiff_t total = size * count;
  uintptr_t start = (uintptr_t)a->current - (uintptr_t)total;

  // logic to align down if needed
  uintptr_t mask = (uintptr_t)(align - 1);
  start &= ~mask;

  uint8_t *start_ptr = (uint8_t *)start;
  if (start_ptr < a->base) {
    abort();
  }
  a->current = start_ptr;
  return memset(start_ptr, 0, (size_t)total);
}
