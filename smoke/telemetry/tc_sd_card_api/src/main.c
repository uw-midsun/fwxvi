/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for sc_sd_card_api
 *
 * @date   2025-10-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#include "delay.h"
#include "ff.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */
#include "sd_card_interface.h"
#include "sd_card_spi.h"

/* SD cards require SPI mode 0 (CPOL=0, CPHA=0) */
SdSpiSettings sd_spi_test_settings = { .baudrate = SD_SPI_BAUDRATE_2_5MHZ,
                                       .mode = SD_SPI_MODE_0,
                                       .mosi = { .port = GPIO_PORT_B, .pin = 15 },
                                       .miso = { .port = GPIO_PORT_B, .pin = 14 },
                                       .sclk = { .port = GPIO_PORT_B, .pin = 13 },
                                       .cs = { .port = GPIO_PORT_B, .pin = 12 } };

SdSpiPort spi = SD_SPI_PORT_2;

static const char s_test_msg[] = "Hello World!";
#define TEST_MSG_LEN (sizeof(s_test_msg) - 1U)

TASK(sd_card_api, TASK_STACK_1024) {
  FATFS workArea;
  FIL file;
  FRESULT result;
  UINT bytesWritten;
  UINT bytesRead;
  char readBuf[TEST_MSG_LEN];

  LOG_DEBUG("Linking driver...\r\n");
  if (sd_card_link_driver(spi, &sd_spi_test_settings) != STATUS_CODE_OK) {
    LOG_DEBUG("Driver linking failure\r\n");
    goto error;
  }
  LOG_DEBUG("Driver linked\r\n");

  result = f_mount(&workArea, "", 1);
  if (result != FR_OK) {
    LOG_DEBUG("f_mount() failed, result = %d\r\n", result);
    goto error;
  }
  LOG_DEBUG("Workarea mounted\r\n");

  /* Write phase */
  result = f_open(&file, "test.txt", FA_WRITE | FA_CREATE_ALWAYS);
  if (result != FR_OK) {
    LOG_DEBUG("f_open(write) failed, result = %d\r\n", result);
    goto error;
  }
  LOG_DEBUG("Opened test.txt for writing\r\n");

  result = f_write(&file, s_test_msg, TEST_MSG_LEN, &bytesWritten);
  if (result != FR_OK || bytesWritten != TEST_MSG_LEN) {
    LOG_DEBUG("f_write() failed, result = %d, written = %u\r\n", result, bytesWritten);
    f_close(&file);
    goto error;
  }
  LOG_DEBUG("Wrote %u bytes\r\n", bytesWritten);

  result = f_close(&file);
  if (result != FR_OK) {
    LOG_DEBUG("f_close(write) failed, result = %d\r\n", result);
    goto error;
  }
  LOG_DEBUG("File closed\r\n");

  /* Read-back phase - verifies the data actually landed on the card */
  result = f_open(&file, "test.txt", FA_READ);
  if (result != FR_OK) {
    LOG_DEBUG("f_open(read) failed, result = %d\r\n", result);
    goto error;
  }

  result = f_read(&file, readBuf, TEST_MSG_LEN, &bytesRead);
  if (result != FR_OK || bytesRead != TEST_MSG_LEN) {
    LOG_DEBUG("f_read() failed, result = %d, read = %u\r\n", result, bytesRead);
    f_close(&file);
    goto error;
  }

  result = f_close(&file);
  if (result != FR_OK) {
    LOG_DEBUG("f_close(read) failed, result = %d\r\n", result);
    goto error;
  }

  if (memcmp(readBuf, s_test_msg, TEST_MSG_LEN) != 0) {
    LOG_DEBUG("Read-back mismatch\r\n");
    goto error;
  }
  LOG_DEBUG("Read-back verified: %.*s\r\n", (int)TEST_MSG_LEN, readBuf);

  result = f_unmount("");
  if (result != FR_OK) {
    LOG_DEBUG("f_unmount() failed, result = %d\r\n", result);
    goto error;
  }
  LOG_DEBUG("Workarea unmounted\r\n");

  while (true) {
    LOG_DEBUG("sd_card_api PASS\r\n");
    delay_ms(1000U);
  }

error:
  while (true) {
    LOG_DEBUG("sd_card_api FAIL\r\n");
    delay_ms(1000U);
  }
}

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
int main(int argc, char *argv[]) {
  mpxe_init(argc, argv);
#else
int main() {
#endif
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(sd_card_api, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
