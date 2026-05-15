/************************************************************************************************
 * @file   test_bootstrap.c
 *
 * @brief  Test file for bootstrap
 *
 * @date   2025-07-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "bootstrap.h"

void setup_test(void) {
  StatusCode status = fs_init();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
}

void teardown_test(void) {}

void test_compute_crc32_known_vector(void) {
  const uint8_t data[] = "123456789";
  uint32_t crc = 0U;

  crc32_init();
  compute_crc32(data, sizeof(data) - 1U, &crc);

  TEST_ASSERT_EQUAL_HEX32(0xCBF43926U, crc);
}

void test_parse_crc32_hex_valid_uppercase(void) {
  const uint8_t crc_text[] = "CBF43926";
  uint32_t crc = 0U;

  StatusCode status = parse_crc32_hex(crc_text, sizeof(crc_text) - 1U, &crc);

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL_HEX32(0xCBF43926U, crc);
}

void test_parse_crc32_hex_valid_lowercase(void) {
  const uint8_t crc_text[] = "cbf43926";
  uint32_t crc = 0U;

  StatusCode status = parse_crc32_hex(crc_text, sizeof(crc_text) - 1U, &crc);

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL_HEX32(0xCBF43926U, crc);
}

void test_parse_crc32_hex_invalid_character(void) {
  const uint8_t crc_text[] = "CBF43Z26";
  uint32_t crc = 0U;

  StatusCode status = parse_crc32_hex(crc_text, sizeof(crc_text) - 1U, &crc);

  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, status);
}

void test_read_crc32_from_filesystem(void) {
  const uint8_t crc_text[] = "CBF43926";
  uint32_t crc = 0U;

  StatusCode status = fs_add_file(CRC_FILE_PATH, crc_text, sizeof(crc_text) - 1U, FILETYPE_FILE);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);

  status = read_crc32(CRC_FILE_PATH, CRC32_HEX_STRING_SIZE, &crc);

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_EQUAL_HEX32(0xCBF43926U, crc);
}
