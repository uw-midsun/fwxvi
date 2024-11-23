#pragma once

#include <stddef.h>
#include <stdint.h>

void crc16_init_table(void); 
uint32_t calculate_crc16(const uint8_t *data, size_t length);