/************************************************************************************************
 * @file   test_telemetry.c
 *
 * @brief  Test file for telemetry
 *
 * @date   2025-01-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "queues.h"
#include "test_helpers.h"
#include "uart.h"
#include "unity.h"

/* Intra-component Headers */
#include "imu.h"
#include "sd_card_interface.h"
#include "telemetry.h"

extern Disk_drvTypeDef disk;

void setup_test(void) {}

void teardown_test(void) {}

void test_telemetry_rejects_missing_startup_inputs(void) {
  TelemetryStorage storage = { 0 };
  TelemetryConfig config = { 0 };
  Bmi323Settings settings = { 0 };
  Bmi323Storage imu = { .settings = &settings };
  CanStorage can = { 0 };

  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, telemetry_init(NULL, &config, &imu, &can));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, telemetry_init(&storage, NULL, &imu, &can));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, telemetry_init(&storage, &config, NULL, &can));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, telemetry_init(&storage, &config, &imu, NULL));
  imu.settings = NULL;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, telemetry_init(&storage, &config, &imu, &can));
  TEST_ASSERT_NULL(storage.config);
}

/* Keep the singleton registration lifecycle in one test: there is no unlink API. */
void test_sd_registration_lifecycle(void) {
  static SdSpiSettings settings;
  TEST_ASSERT_EQUAL(FR_INVALID_DRIVE, sd_card_mount());
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, sd_card_link_driver(SD_SPI_PORT_2, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, sd_card_link_driver((SdSpiPort)-1, &settings));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, sd_card_link_driver(NUM_SD_SPI_PORTS, &settings));
  TEST_ASSERT_EQUAL_UINT8(0, FATFS_GetAttachedDriversNbr());

  /* Force FatFs registration failure, then verify the singleton can retry. */
  disk.nbr = FF_VOLUMES;
  TEST_ASSERT_EQUAL(STATUS_CODE_INTERNAL_ERROR, sd_card_link_driver(SD_SPI_PORT_2, &settings));
  TEST_ASSERT_EQUAL(FR_INVALID_DRIVE, sd_card_mount());
  disk.nbr = 0;
  TEST_ASSERT_OK(sd_card_link_driver(SD_SPI_PORT_2, &settings));
  TEST_ASSERT_EQUAL_UINT8(1, FATFS_GetAttachedDriversNbr());
  TEST_ASSERT_EQUAL(STATUS_CODE_ALREADY_INITIALIZED, sd_card_link_driver(SD_SPI_PORT_2, &settings));
  TEST_ASSERT_EQUAL(STATUS_CODE_ALREADY_INITIALIZED, sd_card_link_driver(SD_SPI_PORT_1, &settings));
  TEST_ASSERT_EQUAL_UINT8(1, FATFS_GetAttachedDriversNbr());
}

void test_imu_rejects_sampling_before_successful_initialization(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_UNINITIALIZED, imu_run());
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, bmi323_update(NULL));
  Bmi323Storage storage = { 0 };
  Bmi323Settings settings = { 0 };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, bmi323_init(&storage));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, imu_init(NULL, &settings));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, imu_init(&storage, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_UNINITIALIZED, imu_run());
}

TEST_IN_TASK
void test_sd_failed_mount_keeps_disk_unready(void) {
  /* The host has no card bytes: transport failure must not become success. */
  TEST_ASSERT_EQUAL(FR_NOT_READY, sd_card_mount());
  BYTE buffer[512] = { 0 };
  LBA_t capacity = 123U;
  const Diskio_drvTypeDef *driver = disk.drv[0];
  TEST_ASSERT_EQUAL(STA_NOINIT, driver->disk_status(0));
  TEST_ASSERT_EQUAL(RES_NOTRDY, driver->disk_read(0, buffer, 0, 1));
  TEST_ASSERT_EQUAL(RES_NOTRDY, driver->disk_write(0, buffer, 0, 1));
  TEST_ASSERT_EQUAL(RES_NOTRDY, driver->disk_ioctl(0, CTRL_SYNC, NULL));
  TEST_ASSERT_EQUAL(RES_NOTRDY, driver->disk_ioctl(0, GET_SECTOR_COUNT, &capacity));
  TEST_ASSERT_EQUAL_UINT32(123U, capacity);
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, sd_spi_cs_get_state(SD_SPI_PORT_2));
}

TEST_IN_TASK
void test_datagram_queue_round_trip(void) {
  static TelemetryStorage storage;
  /* Exercise the same statically backed queue layout used by telemetry_init. */
  storage.datagram_queue = (Queue){ .num_items = DATAGRAM_BUFFER_SIZE, .item_size = sizeof(Datagram), .storage_buf = (uint8_t *)storage.datagram_buffer };
  TEST_ASSERT_OK(queue_init(&storage.datagram_queue));
  Datagram sent = { .start_frame = DATAGRAM_START_FRAME, .id = 0x123U, .dlc = 3U, .data = { 0x12U, 0x34U, 0x56U, DATAGRAM_END_FRAME } }, received;
  TEST_ASSERT_OK(queue_send(&storage.datagram_queue, &sent, 0U));
  TEST_ASSERT_OK(queue_receive(&storage.datagram_queue, &received, 0U));
  TEST_ASSERT_EQUAL_MEMORY(&sent, &received, sizeof(sent));
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, queue_receive(&storage.datagram_queue, &received, 0U));
}
