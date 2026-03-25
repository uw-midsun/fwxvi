/************************************************************************************************
 * @file   test_fota_packet.c
 *
 * @brief  Unit tests for fota_packet.c
 *
 * @date   2025-07-05
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "fota_packet.h"

static FotaPacket s_packet;

void setup_test(void) {
  memset(&s_packet, 0, sizeof(FotaPacket));
}

void teardown_test(void) {}

TEST_IN_TASK
void test_fota_packet_init_valid(void) {
  FotaError err = fota_packet_init(&s_packet, FOTA_PACKET_TYPE_DATA, 1, 16);
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, err);
  TEST_ASSERT_EQUAL_UINT8(FOTA_PACKET_SOF, s_packet.sof);
  TEST_ASSERT_EQUAL(FOTA_PACKET_TYPE_DATA, s_packet.packet_type);
  TEST_ASSERT_EQUAL_UINT8(1, s_packet.sequence_num);
  TEST_ASSERT_EQUAL_UINT16(16, s_packet.payload_length);
  TEST_ASSERT_EQUAL_UINT32(0, s_packet.crc32);
  TEST_ASSERT_EQUAL_UINT8(FOTA_PACKET_EOF, s_packet.eof);

  for (uint16_t i = 0U; i < FOTA_PACKET_PAYLOAD_SIZE; ++i) {
    TEST_ASSERT_EQUAL_UINT8(0, s_packet.payload[i]);
  }
}

TEST_IN_TASK
void test_fota_packet_init_invalid_args(void) {
  /* Null packet pointer */
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, fota_packet_init(NULL, FOTA_PACKET_TYPE_DATA, 0, 16));

  /* Payload length too large */
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, fota_packet_init(&s_packet, FOTA_PACKET_TYPE_DATA, 0, FOTA_PACKET_PAYLOAD_SIZE + 4));

  /* Payload length exactly 0 is valid? Let's check 0-length allowed */
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_packet_init(&s_packet, FOTA_PACKET_TYPE_DATA, 0, 0));
}

TEST_IN_TASK
void test_fota_packet_set_crc_valid(void) {
  fota_packet_init(&s_packet, FOTA_PACKET_TYPE_DATA, 1, 8);

  /* Fill first 8 bytes with known pattern */
  for (uint8_t i = 0; i < 8; ++i) s_packet.payload[i] = i;

  /* Ensure rest of payload is zeroed */
  for (uint16_t i = 8; i < FOTA_PACKET_PAYLOAD_SIZE; ++i) s_packet.payload[i] = 0;

  FotaError err = fota_packet_set_crc(&s_packet);
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, err);
  TEST_ASSERT_NOT_EQUAL(0, s_packet.crc32);

  /* If payload changes, CRC changes (basic check) */
  uint32_t old_crc = s_packet.crc32;
  s_packet.payload[0] ^= 0xFF;
  fota_packet_set_crc(&s_packet);
  TEST_ASSERT_NOT_EQUAL(old_crc, s_packet.crc32);
}

TEST_IN_TASK
void test_fota_packet_set_crc_null(void) {
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, fota_packet_set_crc(NULL));
}

TEST_IN_TASK
void test_fota_packet_serialize_and_deserialize_roundtrip(void) {
  fota_packet_init(&s_packet, FOTA_PACKET_TYPE_DATA, 5, 12);

  /* Fill payload with incremental values */
  for (uint8_t i = 0; i < 12; i++) s_packet.payload[i] = i;
  /* Zero pad the rest */
  for (uint16_t i = 12; i < FOTA_PACKET_PAYLOAD_SIZE; ++i) s_packet.payload[i] = 0;

  fota_packet_set_crc(&s_packet);

  uint8_t buffer[FOTA_PACKET_SERIALIZED_SIZE] = { 0 };
  uint32_t bytes_written = 0;
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_packet_serialize(&s_packet, buffer, sizeof(buffer), &bytes_written));
  TEST_ASSERT_EQUAL_UINT32(FOTA_PACKET_SERIALIZED_SIZE, bytes_written);

  TEST_ASSERT_EQUAL_UINT8(FOTA_PACKET_SOF, buffer[0]);
  TEST_ASSERT_EQUAL_UINT8(FOTA_PACKET_EOF, buffer[bytes_written - 1]);

  FotaPacket deserialized = { 0 };
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_packet_deserialize(&deserialized, buffer, bytes_written));

  /* Compare all struct fields (except maybe CRC)  */
  TEST_ASSERT_EQUAL_UINT8(s_packet.sof, deserialized.sof);
  TEST_ASSERT_EQUAL(s_packet.packet_type, deserialized.packet_type);
  TEST_ASSERT_EQUAL_UINT32(s_packet.datagram_id, deserialized.datagram_id);
  TEST_ASSERT_EQUAL_UINT8(s_packet.sequence_num, deserialized.sequence_num);
  TEST_ASSERT_EQUAL_UINT16(s_packet.payload_length, deserialized.payload_length);
  TEST_ASSERT_EQUAL_UINT32(s_packet.crc32, deserialized.crc32);
  TEST_ASSERT_EQUAL_UINT8(s_packet.eof, deserialized.eof);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(s_packet.payload, deserialized.payload, FOTA_PACKET_PAYLOAD_SIZE);
}

TEST_IN_TASK
void test_fota_packet_serialize_invalid_args(void) {
  uint8_t buf[FOTA_PACKET_SERIALIZED_SIZE] = { 0 };
  uint32_t written = 0;
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, fota_packet_serialize(NULL, buf, sizeof(buf), &written));
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, fota_packet_serialize(&s_packet, NULL, sizeof(buf), &written));
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, fota_packet_serialize(&s_packet, buf, sizeof(buf), NULL));
}

TEST_IN_TASK
void test_fota_packet_serialize_buffer_too_small(void) {
  fota_packet_init(&s_packet, FOTA_PACKET_TYPE_DATA, 2, 64);
  uint8_t buffer[16] = { 0 };
  uint32_t bytes_written = 0;
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, fota_packet_serialize(&s_packet, buffer, sizeof(buffer), &bytes_written));
}

TEST_IN_TASK
void test_fota_packet_deserialize_invalid(void) {
  FotaPacket pkt;
  uint8_t too_small[5] = { 0 };
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, fota_packet_deserialize(&pkt, NULL, 10));
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, fota_packet_deserialize(NULL, too_small, 10));
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, fota_packet_deserialize(&pkt, too_small, 5));
}

TEST_IN_TASK
void test_fota_packet_deserialize_invalid_payload_length(void) {
  uint8_t buffer[FOTA_PACKET_SERIALIZED_SIZE] = { 0 };
  buffer[0] = FOTA_PACKET_SOF;
  buffer[1] = FOTA_PACKET_TYPE_DATA;
  buffer[6] = 1;
  buffer[7] = 255;
  buffer[8] = 0;
  buffer[FOTA_PACKET_SERIALIZED_SIZE - 1] = FOTA_PACKET_EOF;
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_PACKET, fota_packet_deserialize(&s_packet, buffer, sizeof(buffer)));
}

TEST_IN_TASK
void test_fota_packet_deserialize_invalid_sof_eof(void) {
  fota_packet_init(&s_packet, FOTA_PACKET_TYPE_DATA, 0, 8);
  for (uint8_t i = 0; i < 8; ++i) s_packet.payload[i] = i;
  for (uint16_t i = 8; i < FOTA_PACKET_PAYLOAD_SIZE; ++i) s_packet.payload[i] = 0;
  fota_packet_set_crc(&s_packet);

  uint8_t buffer[FOTA_PACKET_SERIALIZED_SIZE] = { 0 };
  uint32_t bytes_written = 0;
  fota_packet_serialize(&s_packet, buffer, sizeof(buffer), &bytes_written);

  buffer[0] = 0x00;
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_PACKET, fota_packet_deserialize(&s_packet, buffer, bytes_written));

  buffer[0] = FOTA_PACKET_SOF;
  buffer[bytes_written - 1] = 0x00;
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_PACKET, fota_packet_deserialize(&s_packet, buffer, bytes_written));
}

TEST_IN_TASK
void test_fota_packet_zero_and_full_payload_length(void) {
  FotaError err = fota_packet_init(&s_packet, FOTA_PACKET_TYPE_DATA, 0, 0);
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, err);
  TEST_ASSERT_EQUAL_UINT16(0, s_packet.payload_length);

  err = fota_packet_init(&s_packet, FOTA_PACKET_TYPE_DATA, 0, FOTA_PACKET_PAYLOAD_SIZE);
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, err);
  TEST_ASSERT_EQUAL_UINT16(FOTA_PACKET_PAYLOAD_SIZE, s_packet.payload_length);
}

TEST_IN_TASK
void test_fota_packet_payload_padding_is_zeroed(void) {
  memset(s_packet.payload, 0xFF, FOTA_PACKET_PAYLOAD_SIZE);
  fota_packet_init(&s_packet, FOTA_PACKET_TYPE_DATA, 0, 12);

  for (uint16_t i = 0; i < FOTA_PACKET_PAYLOAD_SIZE; ++i) {
    TEST_ASSERT_EQUAL_UINT8(0, s_packet.payload[i]);
  }
}
