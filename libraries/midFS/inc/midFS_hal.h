#include "status.h"
#include <stdint.h>
#include "stm32l433xx.h"
#include "stm32l4xx_hal_flash.h"

/**
 * Reads memory from HAL of size buffer_len at a specific address to an array buffer
 * 
 */
StatusCode fs_hal_read(uint32_t address, uint8_t *buffer, size_t buffer_len);

/**
 * Writes memory from array buffer of size buffer_len to a specific address in HAL
 * 
 */
StatusCode fs_hal_write(uint32_t address, uint8_t *buffer, size_t buffer_len);