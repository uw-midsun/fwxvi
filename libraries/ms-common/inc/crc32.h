#pragma once

#include <stddef.h>
#include <stdint.h>

void crc32_init_table(void); 
uint32_t calculate_crc(const uint8_t *data, size_t length);