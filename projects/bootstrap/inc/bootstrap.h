#include "midFS.h"
#include <stdint.h>
#include <string.h>

#define BOOTLOADER_ADDR     0x08001000
#define BOOTLOADER_SIZE     0x00007000
#define APPLICATION_ADDR    0x08008000
#define CRC_FILE_PATH       "bootloader.crc"

typedef void (*EntryPoint) (void);

/**
 * Jumps to a specific address in memory
 * 
 */
void jump_to(uint32_t addr);

/**
 * Manually computes CRC
 * 
 */
void compute_crc32(const uint8_t *data, size_t length, uint32_t *crc_dest);


/**
 * Reads CRC value from file system
 * 
 */
void read_crc32(const char *file_path, size_t length, uint32_t *crc_dest);


/**
 * Main application executed at runtime
 * 
 */
void boostrap_main(void);