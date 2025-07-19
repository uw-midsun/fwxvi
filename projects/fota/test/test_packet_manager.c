/************************************************************************************************
 * @file   test_packet_manager.c
 *
 * @brief  Unit tests for packet_manager.c
 *
 * @date   2025-07-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* Inter-component Headers */
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "fota_datagram.h"
#include "fota_packet.h"
#include "network.h"
#include "packet_manager.h"

static PacketManager s_manager;

// Deep copy storage for datagram received in callback
static FotaDatagram s_copied_datagram;
static bool s_copied_datagram_valid = false;

static uint8_t s_test_data[FOTA_MAX_DATAGRAM_SIZE];
static FotaPacket s_packets[FOTA_MAX_PACKETS_PER_DATAGRAM];
static FotaDatagram *s_last_received_datagram = NULL;

void datagram_callback(FotaDatagram *datagram) {
  memcpy(&s_copied_datagram, datagram, sizeof(FotaDatagram));
  s_last_received_datagram = &s_copied_datagram;
  s_copied_datagram_valid = true;
}

void setup_test(void) {
  memset(&s_manager, 0, sizeof(s_manager));
  memset(s_test_data, 0xAB, sizeof(s_test_data));
  memset(s_packets, 0, sizeof(s_packets));
  s_last_received_datagram = NULL;
  s_copied_datagram_valid = false;

  UartSettings settings = {
    .tx = { GPIO_PORT_A, 0 },
    .rx = { GPIO_PORT_A, 1 },
    .baudrate = 115200,
    .flow_control = UART_FLOW_CONTROL_NONE,
  };

  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, packet_manager_init(&s_manager, UART_PORT_2, &settings, datagram_callback));
}

void teardown_test(void) {}

/* Test: Reassemble a valid datagram */
TEST_IN_TASK
void test_packet_manager_reassembles_valid_datagram(void) {
  FotaDatagram datagram;
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_init(&datagram, FOTA_DATAGRAM_TYPE_FIRMWARE_CHUNK, 0x5555, s_test_data, 128));

  uint16_t num_packets = 0;
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_to_packets(&datagram, s_packets, &num_packets, FOTA_MAX_PACKETS_PER_DATAGRAM));

  for (uint16_t i = 0; i < num_packets; i++) {
    uint8_t serialized[FOTA_PACKET_SERIALIZED_SIZE];
    uint32_t serialized_len = 0;
    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_packet_serialize(&s_packets[i], serialized, sizeof(serialized), &serialized_len));

    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, network_sim_set_rx_data(serialized, serialized_len));

    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, packet_manager_process(&s_manager));
  }

  TEST_ASSERT_TRUE(s_copied_datagram_valid);
  TEST_ASSERT_NOT_NULL(s_last_received_datagram);
  TEST_ASSERT_TRUE(fota_datagram_is_complete(s_last_received_datagram));
  TEST_ASSERT_EQUAL_MEMORY(s_test_data, s_last_received_datagram->data, 128);
}

/* Test: Reject datagram if any packet has invalid CRC */
TEST_IN_TASK
void test_packet_manager_rejects_invalid_crc(void) {
  FotaDatagram datagram;
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_init(&datagram, FOTA_DATAGRAM_TYPE_ACKNOWLEDGEMENT, 0x1234, s_test_data, 64));

  uint16_t num_packets = 0;
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_to_packets(&datagram, s_packets, &num_packets, FOTA_MAX_PACKETS_PER_DATAGRAM));

  // Corrupt the payload and update CRC of second packet to simulate failure
  fota_packet_set_crc(&s_packets[1]);
  s_packets[1].payload[0] ^= 0xFF;  // Corrupt payload
  fota_packet_set_crc(&s_packets[1]);

  for (uint16_t i = 0; i < num_packets; i++) {
    uint8_t serialized[FOTA_PACKET_SERIALIZED_SIZE];
    uint32_t serialized_len = 0;
    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_packet_serialize(&s_packets[i], serialized, sizeof(serialized), &serialized_len));

    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, network_sim_set_rx_data(serialized, serialized_len));

    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, packet_manager_process(&s_manager));
  }

  TEST_ASSERT_FALSE(s_copied_datagram_valid);
  TEST_ASSERT_NULL(s_last_received_datagram);  // callback not invoked
}

/* Test: Handle multiple datagrams back to back */
TEST_IN_TASK
void test_packet_manager_handles_multiple_back_to_back_datagrams(void) {
  for (uint32_t i = 0; i < 2; ++i) {
    memset(s_test_data, i, sizeof(s_test_data));
    FotaDatagram datagram;
    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_init(&datagram, FOTA_DATAGRAM_TYPE_FIRMWARE_METADATA, 0xBEEF + i, s_test_data, 100));

    uint16_t num_packets = 0;
    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_to_packets(&datagram, s_packets, &num_packets, FOTA_MAX_PACKETS_PER_DATAGRAM));

    s_copied_datagram_valid = false;  // Reset flag for each datagram

    for (uint16_t j = 0; j < num_packets; ++j) {
      uint8_t serialized[FOTA_PACKET_SERIALIZED_SIZE];
      uint32_t serialized_len = 0;
      TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_packet_serialize(&s_packets[j], serialized, sizeof(serialized), &serialized_len));

      TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, network_sim_set_rx_data(serialized, serialized_len));

      TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, packet_manager_process(&s_manager));
    }

    TEST_ASSERT_TRUE(s_copied_datagram_valid);
    TEST_ASSERT_NOT_NULL(s_last_received_datagram);
    TEST_ASSERT_EQUAL(0xBEEF + i, s_last_received_datagram->header.datagram_id);
    TEST_ASSERT_TRUE(fota_datagram_is_complete(s_last_received_datagram));
  }
}

/* Test: Ignore partial packets (less than full size) */
TEST_IN_TASK
void test_packet_manager_ignores_partial_packet(void) {
  FotaDatagram datagram;
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_init(&datagram, FOTA_DATAGRAM_TYPE_ERROR, 0xCAFE, s_test_data, 80));

  uint8_t serialized[FOTA_PACKET_SERIALIZED_SIZE];
  uint32_t serialized_len = 0;
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_packet_serialize(&s_packets[0], serialized, sizeof(serialized), &serialized_len));

  // Send only half of the packet
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, network_sim_set_rx_data(serialized, serialized_len / 2));
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, packet_manager_process(&s_manager));

  TEST_ASSERT_FALSE(s_copied_datagram_valid);
  TEST_ASSERT_NULL(s_last_received_datagram);
}

/* Test: Ensure invalid arguments return error */
TEST_IN_TASK
void test_packet_manager_returns_invalid_args(void) {
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, packet_manager_process(NULL));

  PacketManager *null_mgr = NULL;
  UartSettings dummy = { 0 };
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, packet_manager_init(null_mgr, UART_PORT_2, &dummy, datagram_callback));
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, packet_manager_init(&s_manager, UART_PORT_2, NULL, datagram_callback));
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, packet_manager_init(&s_manager, NUM_UART_PORTS, &dummy, datagram_callback));
}

/* New Test: Callback called multiple times with different datagrams */
TEST_IN_TASK
void test_packet_manager_callback_multiple_datagrams(void) {
  for (uint32_t i = 0; i < 3; ++i) {
    memset(s_test_data, i + 1, sizeof(s_test_data));
    FotaDatagram datagram;
    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_init(&datagram, FOTA_DATAGRAM_TYPE_FIRMWARE_CHUNK, 0x1000 + i, s_test_data, 64));

    uint16_t num_packets = 0;
    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_to_packets(&datagram, s_packets, &num_packets, FOTA_MAX_PACKETS_PER_DATAGRAM));

    s_copied_datagram_valid = false;

    for (uint16_t j = 0; j < num_packets; ++j) {
      uint8_t serialized[FOTA_PACKET_SERIALIZED_SIZE];
      uint32_t serialized_len = 0;
      TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_packet_serialize(&s_packets[j], serialized, sizeof(serialized), &serialized_len));

      TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, network_sim_set_rx_data(serialized, serialized_len));

      TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, packet_manager_process(&s_manager));
    }

    TEST_ASSERT_TRUE(s_copied_datagram_valid);
    TEST_ASSERT_NOT_NULL(s_last_received_datagram);
    TEST_ASSERT_EQUAL(0x1000 + i, s_last_received_datagram->header.datagram_id);
    TEST_ASSERT_TRUE(fota_datagram_is_complete(s_last_received_datagram));
    TEST_ASSERT_EQUAL(s_test_data[0], s_last_received_datagram->data[0]);
  }
}

/* New Test: Empty datagram data length */
TEST_IN_TASK
void test_packet_manager_empty_datagram(void) {
  FotaDatagram datagram;
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_init(&datagram, FOTA_DATAGRAM_TYPE_FIRMWARE_CHUNK, 0x9999, NULL, 0));

  uint16_t num_packets = 0;
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_datagram_to_packets(&datagram, s_packets, &num_packets, FOTA_MAX_PACKETS_PER_DATAGRAM));

  s_copied_datagram_valid = false;

  for (uint16_t i = 0; i < num_packets; i++) {
    uint8_t serialized[FOTA_PACKET_SERIALIZED_SIZE];
    uint32_t serialized_len = 0;
    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, fota_packet_serialize(&s_packets[i], serialized, sizeof(serialized), &serialized_len));

    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, network_sim_set_rx_data(serialized, serialized_len));

    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, packet_manager_process(&s_manager));
  }

  TEST_ASSERT_TRUE(s_copied_datagram_valid);
  TEST_ASSERT_NOT_NULL(s_last_received_datagram);
  TEST_ASSERT_TRUE(fota_datagram_is_complete(s_last_received_datagram));
  TEST_ASSERT_EQUAL(0, s_last_received_datagram->header.total_length);
}
