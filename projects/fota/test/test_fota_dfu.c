/************************************************************************************************
 * @file   test_fota_dfu.c
 *
 * @brief  Unit tests for fota_dfu.c
 *
 * @date   2025-07-19
 * @author Midnight Sun Team #24
 ************************************************************************************************/

/* Standard headers */
#include <string.h>
#include <stdbool.h>

/* Test Framework */
#include "unity.h"

/* FOTA Components */
#include "fota_dfu.h"
#include "fota_datagram.h"
#include "fota_flash.h"
#include "fota_memory_map.h"
#include "fota_error.h"

/* Helpers */
#include "test_helpers.h"

void setup_test(void) {
}

void teardown_test(void) {}


void test_fota_end_to_end() {
  const uintptr_t staging = APP_STAGING_START_ADDRESS;
  const uintptr_t app_start = APP_ACTIVE_START_ADDRESS;

  PacketManager dummy_pm = {0};
  FotaError err = fota_dfu_init(&dummy_pm, staging, app_start);
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, err);

  // 1. Prepare fake firmware binary
  uint8_t firmware_data[512];
  for (uint32_t i = 0; i < sizeof(firmware_data); i++) {
    firmware_data[i] = (uint8_t)(i ^ 0xAB);  // Arbitrary pattern
  }

  // 2. Calculate expected CRC
  uint32_t expected_crc = fota_calculate_crc32(firmware_data, sizeof(firmware_data) / 4);

  // 3. Send metadata datagram
  FotaDatagram metadata = {0};
  metadata.header.type = FOTA_DATAGRAM_TYPE_FIRMWARE_METADATA;
  metadata.header.total_length = sizeof(FotaDatagramPayload_FirmwareMetadata);
  metadata.is_complete = true;

  FotaDatagramPayload_FirmwareMetadata metadata_payload = {
    .binary_size = sizeof(firmware_data),
    .expected_binary_crc32 = expected_crc,
    .version_major = 1,
    .version_minor = 0,
  };

  memcpy(metadata_payload.firmware_id, "TEST_FW_ID_01", 13);
  memcpy(metadata.data, &metadata_payload, sizeof(metadata_payload));

  err = fota_dfu_process(&metadata);
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, err);

  // 4. Send firmware chunks
  size_t offset = 0;
  uint16_t datagram_id = 0;

  while (offset < sizeof(firmware_data)) {
    size_t chunk_len = (sizeof(firmware_data) - offset > 64) ? 64 : (sizeof(firmware_data) - offset);

    FotaDatagram chunk = {0};
    chunk.header.type = FOTA_DATAGRAM_TYPE_FIRMWARE_CHUNK;
    chunk.header.total_length = chunk_len;
    chunk.header.datagram_id = datagram_id++;
    chunk.is_complete = true;

    FotaDatagramPayload_FirmwareChunk *chunk_payload = (FotaDatagramPayload_FirmwareChunk *)chunk.data;
    memcpy(chunk_payload->data, firmware_data + offset, chunk_len);

    err = fota_dfu_process(&chunk);

    if (err != FOTA_ERROR_SUCCESS && err != FOTA_ERROR_BOOTLOADER_FAILURE) {
      printf("Chunk failed with err: %u\r\n", err);
    }
    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, err);

    offset += chunk_len;
  }

  // 5. Verify application area is updated
  uint8_t verify_buf[sizeof(firmware_data)];
  err = fota_flash_read(app_start, verify_buf, sizeof(verify_buf));
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, err);
  TEST_ASSERT_EQUAL(0, memcmp(verify_buf, firmware_data, sizeof(firmware_data)));
}
