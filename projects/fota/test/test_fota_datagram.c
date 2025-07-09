/************************************************************************************************
 * @file   test_fota_datagram.c
 *
 * @brief  Unit tests for fota_datagram.c
 *
 * @date   2025-07-06
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "fota_datagram.h"
#include "fota_encryption.h"
#include "fota_packet.h"

static FotaDatagram s_datagram;
static FotaDatagram s_datagram_reconstructed;
static FotaPacket s_packets[FOTA_MAX_PACKETS_PER_DATAGRAM];
static uint8_t s_original_data[FOTA_MAX_DATAGRAM_SIZE];

void setup_test(void) {
  memset(&s_datagram, 0, sizeof(s_datagram));
  memset(&s_datagram_reconstructed, 0, sizeof(s_datagram_reconstructed));
  memset(s_packets, 0, sizeof(s_packets));
  for (uint16_t i = 0; i < sizeof(s_original_data); i++) {
    s_original_data[i] = (uint8_t)(i & 0xFF);
  }
}

void teardown_test(void) {}

TEST_IN_TASK
void test_fota_full_pipeline_roundtrip(void) {
  FotaError err = fota_datagram_init(&s_datagram, FOTA_DATAGRAM_TYPE_FIRMWARE_CHUNK, 0xABCD1234, s_original_data, 512);
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, err);

  uint16_t num_packets = 0;
  err = fota_datagram_to_packets(&s_datagram, s_packets, &num_packets, FOTA_MAX_PACKETS_PER_DATAGRAM);
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, err);

  err = fota_datagram_process_header_packet(&s_datagram_reconstructed, &s_packets[0]);
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, err);

  for (uint16_t i = 1; i < num_packets; ++i) {
    err = fota_datagram_process_data_packet(&s_datagram_reconstructed, &s_packets[i]);
    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, err);
  }

  TEST_ASSERT_TRUE(fota_datagram_is_complete(&s_datagram_reconstructed));
  TEST_ASSERT_EQUAL_MEMORY(s_original_data, s_datagram_reconstructed.data, 512);
  TEST_ASSERT_EQUAL(s_datagram.header.datagram_crc32, fota_calculate_crc32(s_datagram_reconstructed.data, s_datagram_reconstructed.header.total_length / 4U));
}

TEST_IN_TASK
void test_fota_max_payload(void) {
  uint8_t max_data[FOTA_MAX_DATAGRAM_SIZE];
  memset(max_data, 0x55, sizeof(max_data));
  FotaError err = fota_datagram_init(&s_datagram, FOTA_DATAGRAM_TYPE_ERROR, 0x9999, max_data, sizeof(max_data));
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, err);

  uint16_t num_packets = 0;
  err = fota_datagram_to_packets(&s_datagram, s_packets, &num_packets, FOTA_MAX_PACKETS_PER_DATAGRAM);
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, err);
  TEST_ASSERT_EQUAL(s_datagram.header.num_packets + 1, num_packets);
}

TEST_IN_TASK
void test_fota_packet_datagram_id_mismatch(void) {
  fota_datagram_init(&s_datagram, FOTA_DATAGRAM_TYPE_FIRMWARE_CHUNK, 0x1234, s_original_data, 64);

  FotaPacket pkt;
  fota_packet_init(&pkt, FOTA_PACKET_TYPE_DATA, 0, 64);
  pkt.datagram_id = 0x5678;
  memcpy(pkt.payload, s_original_data, 64);
  fota_packet_set_crc(&pkt);

  FotaError err = fota_datagram_process_data_packet(&s_datagram, &pkt);
  TEST_ASSERT_EQUAL(FOTA_ERROR_INTERNAL_ERROR, err);
}

TEST_IN_TASK
void test_fota_packet_double_receive(void) {
  fota_datagram_init(&s_datagram, FOTA_DATAGRAM_TYPE_FIRMWARE_CHUNK, 0x1234, s_original_data, 64);

  FotaPacket pkt;
  fota_packet_init(&pkt, FOTA_PACKET_TYPE_DATA, 0, 64);
  pkt.datagram_id = 0x1234;
  memcpy(pkt.payload, s_original_data, 64);
  fota_packet_set_crc(&pkt);

  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_process_data_packet(&s_datagram, &pkt));
  TEST_ASSERT_EQUAL(FOTA_ERROR_INTERNAL_ERROR, fota_datagram_process_data_packet(&s_datagram, &pkt));
}

TEST_IN_TASK
void test_fota_datagram_verify_delegation(void) {
  fota_datagram_init(&s_datagram, FOTA_DATAGRAM_TYPE_FIRMWARE_CHUNK, 42, s_original_data, 128);
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_verify(&s_datagram));
}

TEST_IN_TASK
void test_fota_datagram_crc_mismatch_detection(void) {
  fota_datagram_init(&s_datagram, FOTA_DATAGRAM_TYPE_ACKNOWLEDGEMENT, 0xA1B2, s_original_data, 128);
  uint16_t num_packets = 0;
  fota_datagram_to_packets(&s_datagram, s_packets, &num_packets, FOTA_MAX_PACKETS_PER_DATAGRAM);

  s_packets[1].payload[0] ^= 0xFF;
  s_packets[1].crc32 = fota_calculate_crc32(s_packets[1].payload, s_packets[1].payload_length / 4);

  fota_datagram_process_header_packet(&s_datagram_reconstructed, &s_packets[0]);

  FotaError err = fota_datagram_process_data_packet(&s_datagram_reconstructed, &s_packets[1]);
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, err);

  TEST_ASSERT_NOT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_verify(&s_datagram_reconstructed));
}

TEST_IN_TASK
void test_fota_datagram_reconstruction_out_of_order(void) {
  fota_datagram_init(&s_datagram, FOTA_DATAGRAM_TYPE_ACKNOWLEDGEMENT, 0x2222, s_original_data, 256);
  uint16_t num_packets = 0;
  fota_datagram_to_packets(&s_datagram, s_packets, &num_packets, FOTA_MAX_PACKETS_PER_DATAGRAM);

  fota_datagram_process_header_packet(&s_datagram_reconstructed, &s_packets[0]);

  for (int i = num_packets - 1; i >= 1; i--) {
    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_process_data_packet(&s_datagram_reconstructed, &s_packets[i]));
  }

  TEST_ASSERT_TRUE(fota_datagram_is_complete(&s_datagram_reconstructed));
  TEST_ASSERT_EQUAL_MEMORY(s_datagram.data, s_datagram_reconstructed.data, 256);
}

TEST_IN_TASK
void test_fota_datagram_skipped_packet_index(void) {
  fota_datagram_init(&s_datagram, FOTA_DATAGRAM_TYPE_ACKNOWLEDGEMENT, 0x4321, s_original_data, 256);
  uint16_t num_packets = 0;
  fota_datagram_to_packets(&s_datagram, s_packets, &num_packets, FOTA_MAX_PACKETS_PER_DATAGRAM);

  fota_datagram_process_header_packet(&s_datagram_reconstructed, &s_packets[0]);

  for (uint16_t i = 1; i < num_packets; ++i) {
    if (i == 2) continue;
    fota_datagram_process_data_packet(&s_datagram_reconstructed, &s_packets[i]);
  }

  TEST_ASSERT_FALSE(fota_datagram_is_complete(&s_datagram_reconstructed));
  TEST_ASSERT_NOT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_verify(&s_datagram_reconstructed));
}

TEST_IN_TASK
void test_fota_multiple_header_packets_received(void) {
  fota_datagram_init(&s_datagram, FOTA_DATAGRAM_TYPE_JUMP_TO_APP, 0xBEAD, s_original_data, 64);
  uint16_t num_packets = 0;
  fota_datagram_to_packets(&s_datagram, s_packets, &num_packets, FOTA_MAX_PACKETS_PER_DATAGRAM);

  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_process_header_packet(&s_datagram_reconstructed, &s_packets[0]));
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_process_header_packet(&s_datagram_reconstructed, &s_packets[0]));
}

TEST_IN_TASK
void test_fota_datagram_overflow_packet_count(void) {
  fota_datagram_init(&s_datagram, FOTA_DATAGRAM_TYPE_FIRMWARE_CHUNK, 0xDEAD, s_original_data, FOTA_MAX_DATAGRAM_SIZE);
  uint16_t num_packets = 0;
  fota_datagram_to_packets(&s_datagram, s_packets, &num_packets, FOTA_MAX_PACKETS_PER_DATAGRAM);

  fota_datagram_process_header_packet(&s_datagram_reconstructed, &s_packets[0]);

  for (uint16_t i = 1; i < num_packets; ++i) {
    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_process_data_packet(&s_datagram_reconstructed, &s_packets[i]));
  }

  FotaPacket extra;
  fota_packet_init(&extra, FOTA_PACKET_TYPE_DATA, 99, 8);
  extra.datagram_id = s_datagram.header.datagram_id;
  extra.payload[0] = 0xFF;
  fota_packet_set_crc(&extra);

  TEST_ASSERT_EQUAL(FOTA_ERROR_INTERNAL_ERROR, fota_datagram_process_data_packet(&s_datagram_reconstructed, &extra));
}

TEST_IN_TASK
void test_fota_multiple_complete_datagrams_back_to_back(void) {
  for (int d = 0; d < 2; ++d) {
    memset(&s_datagram, 0, sizeof(s_datagram));
    memset(&s_datagram_reconstructed, 0, sizeof(s_datagram_reconstructed));
    memset(s_packets, 0, sizeof(s_packets));

    for (uint16_t i = 0; i < sizeof(s_original_data); ++i) {
      s_original_data[i] = (uint8_t)((i + d) & 0xFF);
    }

    uint16_t num_packets = 0;
    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_init(&s_datagram, FOTA_DATAGRAM_TYPE_FIRMWARE_METADATA, 100 + d, s_original_data, 300));
    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_to_packets(&s_datagram, s_packets, &num_packets, FOTA_MAX_PACKETS_PER_DATAGRAM));

    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_process_header_packet(&s_datagram_reconstructed, &s_packets[0]));

    for (uint16_t i = 1; i < num_packets; ++i) {
      TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_process_data_packet(&s_datagram_reconstructed, &s_packets[i]));
    }

    TEST_ASSERT_TRUE(fota_datagram_is_complete(&s_datagram_reconstructed));
    TEST_ASSERT_EQUAL_MEMORY(s_original_data, s_datagram_reconstructed.data, 300);
    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_verify(&s_datagram_reconstructed));
  }
}

TEST_IN_TASK
void test_fota_incomplete_data_then_recover(void) {
  fota_datagram_init(&s_datagram, FOTA_DATAGRAM_TYPE_ACKNOWLEDGEMENT, 0x5555, s_original_data, 300);
  uint16_t num_packets = 0;
  fota_datagram_to_packets(&s_datagram, s_packets, &num_packets, FOTA_MAX_PACKETS_PER_DATAGRAM);

  fota_datagram_process_header_packet(&s_datagram_reconstructed, &s_packets[0]);

  for (uint16_t i = 1; i < num_packets - 1; ++i) {
    fota_datagram_process_data_packet(&s_datagram_reconstructed, &s_packets[i]);
  }

  TEST_ASSERT_FALSE(fota_datagram_is_complete(&s_datagram_reconstructed));

  fota_datagram_process_data_packet(&s_datagram_reconstructed, &s_packets[num_packets - 1]);
  TEST_ASSERT_TRUE(fota_datagram_is_complete(&s_datagram_reconstructed));
}

TEST_IN_TASK
void test_fota_null_inputs_are_rejected(void) {
  FotaError err;
  err = fota_datagram_init(NULL, FOTA_DATAGRAM_TYPE_ACKNOWLEDGEMENT, 0, s_original_data, 64);
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, err);

  err = fota_datagram_to_packets(NULL, s_packets, &(uint16_t){ 0 }, FOTA_MAX_PACKETS_PER_DATAGRAM);
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, err);

  err = fota_datagram_to_packets(&s_datagram, NULL, &(uint16_t){ 0 }, FOTA_MAX_PACKETS_PER_DATAGRAM);
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, err);

  err = fota_datagram_to_packets(&s_datagram, s_packets, NULL, FOTA_MAX_PACKETS_PER_DATAGRAM);
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, err);

  err = fota_datagram_process_data_packet(NULL, &s_packets[1]);
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, err);

  err = fota_datagram_process_data_packet(&s_datagram, NULL);
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, err);

  err = fota_datagram_process_header_packet(NULL, &s_packets[0]);
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, err);

  err = fota_datagram_process_header_packet(&s_datagram, NULL);
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, err);

  TEST_ASSERT_FALSE(fota_datagram_is_complete(NULL));
  err = fota_datagram_verify(NULL);
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, err);
}

TEST_IN_TASK
void test_fota_single_data_packet_missing_many(void) {
  fota_datagram_init(&s_datagram, FOTA_DATAGRAM_TYPE_FIRMWARE_CHUNK, 55, s_original_data, 512);
  uint16_t num_packets = 0;
  fota_datagram_to_packets(&s_datagram, s_packets, &num_packets, FOTA_MAX_PACKETS_PER_DATAGRAM);

  fota_datagram_process_header_packet(&s_datagram_reconstructed, &s_packets[0]);
  fota_datagram_process_data_packet(&s_datagram_reconstructed, &s_packets[num_packets - 1]);

  TEST_ASSERT_FALSE(fota_datagram_is_complete(&s_datagram_reconstructed));
  TEST_ASSERT_NOT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_verify(&s_datagram_reconstructed));
}

TEST_IN_TASK
void test_fota_header_packet_contains_target_node_id(void) {
  s_datagram.header.target_node_id = 0x2A;
  FotaError err = fota_datagram_init(&s_datagram, FOTA_DATAGRAM_TYPE_ACKNOWLEDGEMENT, 0x9876, s_original_data, 128);
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, err);

  uint16_t num_packets = 0;
  err = fota_datagram_to_packets(&s_datagram, s_packets, &num_packets, FOTA_MAX_PACKETS_PER_DATAGRAM);
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, err);

  err = fota_datagram_process_header_packet(&s_datagram_reconstructed, &s_packets[0]);
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, err);
  TEST_ASSERT_EQUAL(0x2A, s_datagram_reconstructed.header.target_node_id);
}
