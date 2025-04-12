/************************************************************************************************
 * @file   test_network_buffer.c
 *
 * @brief  Test file for network buffer
 *
 * @date   2025-04-03
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "network_buffer.h"

/* Mock object for testing */
static NetworkBuffer s_test_buffer;

void setup_test(void) {
  /* Initialize the buffer before each test */
  network_buffer_init(&s_test_buffer);
}

void teardown_test(void) {
  /* Nothing to clean up */
}

TEST_IN_TASK
void test_network_buffer_init(void) {
  /* Test that initialization sets the correct values */
  NetworkBuffer buffer;
  FotaError result = network_buffer_init(&buffer);

  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, result);
  TEST_ASSERT_EQUAL(0U, buffer.num_items);
  TEST_ASSERT_EQUAL(0U, buffer.read_index);
  TEST_ASSERT_EQUAL(0U, buffer.write_index);

  /* Test with NULL pointer */
  result = network_buffer_init(NULL);
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, result);
}

TEST_IN_TASK
void test_network_buffer_empty_full(void) {
  /* Buffer should start empty */
  TEST_ASSERT_TRUE(network_buffer_empty(&s_test_buffer));
  TEST_ASSERT_FALSE(network_buffer_full(&s_test_buffer));
  TEST_ASSERT_EQUAL(0U, network_buffer_num_items(&s_test_buffer));
  TEST_ASSERT_EQUAL(NETWORK_BUFFER_SIZE, network_buffer_num_empty_slots(&s_test_buffer));

  /* Fill the buffer */
  uint8_t data = 0xAAU;
  for (uint16_t i = 0U; i < NETWORK_BUFFER_SIZE; i++) {
    FotaError result = network_buffer_write(&s_test_buffer, &data);
    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, result);
    data++;
  }

  /* Buffer should now be full */
  TEST_ASSERT_FALSE(network_buffer_empty(&s_test_buffer));
  TEST_ASSERT_TRUE(network_buffer_full(&s_test_buffer));
  TEST_ASSERT_EQUAL(NETWORK_BUFFER_SIZE, network_buffer_num_items(&s_test_buffer));
  TEST_ASSERT_EQUAL(0U, network_buffer_num_empty_slots(&s_test_buffer));
}

TEST_IN_TASK
void test_network_buffer_write_read_single_byte(void) {
  uint8_t write_data = 0x42U;
  uint8_t read_data = 0U;

  /* Write to buffer */
  FotaError result = network_buffer_write(&s_test_buffer, &write_data);
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, result);
  TEST_ASSERT_EQUAL(1, network_buffer_num_items(&s_test_buffer));

  /* Read from buffer */
  result = network_buffer_read(&s_test_buffer, &read_data);
  TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, result);
  TEST_ASSERT_EQUAL(write_data, read_data);
  TEST_ASSERT_EQUAL(0U, network_buffer_num_items(&s_test_buffer));
}

TEST_IN_TASK
void test_network_buffer_write_read_multiple_bytes(void) {
  /* Write multiple bytes */
  for (uint8_t i = 0U; i < 10U; i++) {
    uint8_t data = i;
    FotaError result = network_buffer_write(&s_test_buffer, &data);
    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, result);
  }

  TEST_ASSERT_EQUAL(10U, network_buffer_num_items(&s_test_buffer));

  /* Read multiple bytes and verify they come out in the same order */
  for (uint8_t i = 0U; i < 10U; i++) {
    uint8_t data;
    FotaError result = network_buffer_read(&s_test_buffer, &data);
    TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, result);
    TEST_ASSERT_EQUAL(i, data);
  }

  TEST_ASSERT_EQUAL(0U, network_buffer_num_items(&s_test_buffer));
}

TEST_IN_TASK
void test_network_buffer_circular_behavior(void) {
  /* Fill the buffer completely */
  uint8_t data = 0U;
  for (uint16_t i = 0U; i < NETWORK_BUFFER_SIZE; i++) {
    data = (uint8_t)i;
    network_buffer_write(&s_test_buffer, &data);
  }

  /* Read half the buffer */
  uint8_t read_data;
  for (uint16_t i = 0U; i < NETWORK_BUFFER_SIZE / 2U; i++) {
    network_buffer_read(&s_test_buffer, &read_data);
    TEST_ASSERT_EQUAL((uint8_t)i, read_data);
  }

  /* Write more data to wrap around */
  for (uint16_t i = 0U; i < NETWORK_BUFFER_SIZE / 2U; i++) {
    data = (uint8_t)(NETWORK_BUFFER_SIZE + i);
    network_buffer_write(&s_test_buffer, &data);
  }

  /* Verify the buffer is full again */
  TEST_ASSERT_TRUE(network_buffer_full(&s_test_buffer));

  /* Read the remaining original data */
  for (uint16_t i = NETWORK_BUFFER_SIZE / 2U; i < NETWORK_BUFFER_SIZE; i++) {
    network_buffer_read(&s_test_buffer, &read_data);
    TEST_ASSERT_EQUAL((uint8_t)i, read_data);
  }

  /* Read the new data that wrapped around */
  for (uint16_t i = 0U; i < NETWORK_BUFFER_SIZE / 2U; i++) {
    network_buffer_read(&s_test_buffer, &read_data);
    TEST_ASSERT_EQUAL((uint8_t)(NETWORK_BUFFER_SIZE + i), read_data);
  }

  /* Buffer should be empty now */
  TEST_ASSERT_TRUE(network_buffer_empty(&s_test_buffer));
}

TEST_IN_TASK
void test_network_buffer_error_handling(void) {
  uint8_t data = 0xAAU;
  uint8_t read_data;
  FotaError result;

  /* Test writing to a NULL buffer */
  result = network_buffer_write(NULL, &data);
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, result);

  /* Test writing NULL data */
  result = network_buffer_write(&s_test_buffer, NULL);
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, result);

  /* Test reading from a NULL buffer */
  result = network_buffer_read(NULL, &read_data);
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, result);

  /* Test reading to NULL data */
  result = network_buffer_read(&s_test_buffer, NULL);
  TEST_ASSERT_EQUAL(FOTA_ERROR_INVALID_ARGS, result);

  /* Test reading from an empty buffer */
  result = network_buffer_read(&s_test_buffer, &read_data);
  TEST_ASSERT_EQUAL(FOTA_RESOURCE_EXHAUSTED, result);

  /* Fill the buffer */
  for (uint16_t i = 0U; i < NETWORK_BUFFER_SIZE; i++) {
    network_buffer_write(&s_test_buffer, &data);
  }

  /* Test writing to a full buffer */
  result = network_buffer_write(&s_test_buffer, &data);
  TEST_ASSERT_EQUAL(FOTA_RESOURCE_EXHAUSTED, result);
}

TEST_IN_TASK
void test_network_buffer_stability_under_stress(void) {
  uint8_t write_data, read_data;

  /* Test repeatedly filling and emptying the buffer */
  for (uint8_t cycle = 0U; cycle < 5U; cycle++) {
    /* Fill the buffer */
    for (uint16_t i = 0U; i < NETWORK_BUFFER_SIZE; i++) {
      write_data = (uint8_t)(i + cycle);
      FotaError result = network_buffer_write(&s_test_buffer, &write_data);
      TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, result);
    }

    /* Empty the buffer */
    for (uint16_t i = 0U; i < NETWORK_BUFFER_SIZE; i++) {
      FotaError result = network_buffer_read(&s_test_buffer, &read_data);
      TEST_ASSERT_EQUAL(FOTA_ERROR_SUCCESS, result);
      TEST_ASSERT_EQUAL((uint8_t)(i + cycle), read_data);
    }
  }

  /* Test alternating reads and writes */
  for (uint16_t i = 0U; i < NETWORK_BUFFER_SIZE * 2U; i++) {
    /* Write a byte */
    write_data = (uint8_t)i;
    network_buffer_write(&s_test_buffer, &write_data);

    /* Read a byte */
    network_buffer_read(&s_test_buffer, &read_data);
    TEST_ASSERT_EQUAL(write_data, read_data);

    /* Buffer should remain empty */
    TEST_ASSERT_TRUE(network_buffer_empty(&s_test_buffer));
  }
}
