/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for sd_card_api
 *
 * @date   2025-10-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "status.h"
#include "delay.h"
#include "ff.h"

/* Intra-component Headers */
#include "sd_card_interface.h"
#include "sd_card_spi.h"

SdSpiSettings sd_spi_test_settings {
  .baudrate = SD_SPI_BAUDRATE_2_5MHZ,
  .mode = SPI_MODE_3,
  .mosi = { .port = GPIO_PORT_B, .pin = 15 },
  .miso = { .port = GPIO_PORT_B, .pin = 14 },
  .sclk = { .port = GPIO_PORT_B, .pin = 13 },
  .cs = { .port = GPIO_PORT_B, .pin = 12 }
};

SdSpiPort spi = SD_SPI_PORT_2; 

TASK(sd_card_api, TASK_STACK_1024) {
  LOG_DEBUG("Initializing SD SPI port...\n");

  if (sd_spi_init(spi, &sd_spi_test_settings) == STATUS_CODE_OK) {
    LOG_DEBUG("SD SPI port initialized\n");
  } else {
    LOG_DEBUG("SD SPI port initialization failure\n");
  }

  LOG_DEBUG("Linking driver...\n");

  if (sd_card_link_driver(spi, &sd_spi_test_settings) == STATUS_CODE_OK) {
    LOG_DEBUG("Driver linked\n");
  } else {
    LOG_DEBUG("Driver linking failure\n");
  }

  FATFS workArea;

  FIL file;
  FRESULT result;
  UINT bytesWritten;

  result = f_mount(&workArea, "", 0);
  if (result != FR_OK) {
    LOG_DEBUG("f_mount() failed, result = %d\n", result);
    return;
  }

  LOG_DEBUG("Workarea mounted\n");

  result = f_open(&file, "test.txt", FA_WRITE | FA_CREATE_ALWAYS);
  if (result != FR_OK) {
    LOG_DEBUG("f_open() failed, result = %d\n", result);
    return;
  }

  LOG_DEBUG("Created text.txt\n");

  LOG_DEBUG("Writing to file...\n");

  result = f_write(&file, "Hello World!", 12, &bytesWritten);
  if (result != FR_OK) {
    LOG_DEBUG("f_write() failed, result = %d\n", result);
    return;
  }

  LOG_DEBUG("Writing finished\n");

  f_close(&file);

  if (result != FR_OK) {
    LOG_DEBUG("f_close() failed, result = %d\n", result);
    return;
  }

  LOG_DEBUG("File closed\n");

  f_unmount("");
  if (result != FR_OK) {
    LOG_DEBUG("f_unmount() failed, result = %d\n", result);
    return;
  }

  LOG_DEBUG("Workarea unmounted\n");

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