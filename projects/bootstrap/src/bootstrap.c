/************************************************************************************************
 * @file   bootstrap.c
 *
 * @brief  Source file for bootstrap application
 *
 * @date   2025-09-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#ifdef MS_PLATFORM_ARM
#include "cmsis_gcc.h"
#include "stm32l4xx.h"
#endif

#include "bootstrap.h"

/* Intra-component Headers */

static uint32_t s_crc32_table[256];
static bool s_crc32_initialized = false;

#ifdef MS_PLATFORM_ARM
static bool prv_is_vector_table_valid(uint32_t addr) {
  const uint32_t *vector_table = (const uint32_t *)(uintptr_t)addr;
  uint32_t sp = vector_table[0U];
  uint32_t reset_vector = vector_table[1U];

  bool stack_in_sram = (sp >= SRAM_START_ADDRESS) && (sp <= (SRAM_START_ADDRESS + SRAM_SIZE));
  bool reset_in_flash = (reset_vector >= FLASH_START_ADDRESS_LINKERSCRIPT) &&
                         (reset_vector < (FLASH_START_ADDRESS_LINKERSCRIPT + FLASH_SIZE_LINKERSCRIPT));
  bool reset_is_thumb = (reset_vector & 1U) != 0U;

  return stack_in_sram && reset_in_flash && reset_is_thumb;
}
#endif

static StatusCode prv_hex_to_nibble(uint8_t hex, uint8_t *nibble) {
  if (nibble == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (hex >= '0' && hex <= '9') {
    *nibble = hex - '0';
    return STATUS_CODE_OK;
  }

  if (hex >= 'A' && hex <= 'F') {
    *nibble = hex - 'A' + 10U;
    return STATUS_CODE_OK;
  }

  if (hex >= 'a' && hex <= 'f') {
    *nibble = hex - 'a' + 10U;
    return STATUS_CODE_OK;
  }

  return STATUS_CODE_INVALID_ARGS;
}

#ifdef MS_PLATFORM_ARM
static void prv_jump_to_application_or_halt(void) {
  if (prv_is_vector_table_valid(APP_ACTIVE_START_ADDRESS)) {
    jump_to(APP_ACTIVE_START_ADDRESS);
  }

  while (true) {
  }
}
#endif

void jump_to(uint32_t addr) {
#ifdef MS_PLATFORM_ARM
  uint32_t sp = *((volatile uint32_t *)addr);
  uint32_t reset_vector = *((volatile uint32_t *)(addr + 4));

  __disable_irq();
  SCB->VTOR = addr;
  __DSB();
  __ISB();
  __set_MSP(sp);
  ((EntryPoint)(uintptr_t)reset_vector)();
#else
  (void)addr;
#endif
}

void crc32_init(void) {
  for (uint32_t i = 0; i < 256; i++) {
    uint32_t crc = i;
    for (int j = 0; j < 8; j++) {
      if (crc & 1) {
        crc = (crc >> 1) ^ CRC32_POLY;
      } else {
        crc >>= 1;
      }
    }
    s_crc32_table[i] = crc;
  }

  s_crc32_initialized = true;
}

void compute_crc32(const uint8_t *data, size_t length, uint32_t *crc_dest) {
  if (data == NULL || crc_dest == NULL) {
    return;
  }

  if (!s_crc32_initialized) {
    crc32_init();
  }

  uint32_t crc = 0xFFFFFFFF;

  for (size_t i = 0; i < length; i++) {
    uint8_t byte = data[i];
    crc = (crc >> 8) ^ s_crc32_table[(crc ^ byte) & 0xFF];
  }

  *crc_dest = ~crc;
}

StatusCode parse_crc32_hex(const uint8_t *crc_bytes, size_t length, uint32_t *crc_dest) {
  if (crc_bytes == NULL || crc_dest == NULL || length != CRC32_HEX_STRING_SIZE) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint32_t parsed_crc = 0U;
  for (size_t i = 0U; i < length; i++) {
    uint8_t nibble = 0U;
    status_ok_or_return(prv_hex_to_nibble(crc_bytes[i], &nibble));
    parsed_crc = (parsed_crc << 4U) | nibble;
  }

  *crc_dest = parsed_crc;
  return STATUS_CODE_OK;
}

StatusCode read_crc32(const char *file_path, size_t length, uint32_t *crc_dest) {
  if (file_path == NULL || crc_dest == NULL || length != CRC32_HEX_STRING_SIZE) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint8_t crc_buffer[CRC32_HEX_STRING_SIZE] = { 0U };
  status_ok_or_return(fs_read_file(file_path, crc_buffer));

  return parse_crc32_hex(crc_buffer, length, crc_dest);
}

void bootstrap_main(void) {
#ifndef MS_PLATFORM_ARM
  printf("Bootstrap runtime is only supported on ARM targets\n\r");
  return;
#else
  StatusCode status = fs_pull();
  if (status != STATUS_CODE_OK) {
    prv_jump_to_application_or_halt();
  }

  uint32_t stored_crc = 0U;
  status = read_crc32(CRC_FILE_PATH, CRC32_HEX_STRING_SIZE, &stored_crc);
  if (status != STATUS_CODE_OK) {
    prv_jump_to_application_or_halt();
  }

  uint32_t computed_crc = 0U;
  compute_crc32((const uint8_t *)(uintptr_t)BOOTLOADER_START_ADDRESS, BOOTLOADER_SIZE, &computed_crc);

  bool bootloader_valid = (computed_crc == stored_crc) && prv_is_vector_table_valid(BOOTLOADER_START_ADDRESS);

  if (bootloader_valid) {
    jump_to(BOOTLOADER_START_ADDRESS);
  }

  prv_jump_to_application_or_halt();
#endif
}
