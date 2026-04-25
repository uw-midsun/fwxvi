/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for spi_api
 *
 * @date   2025-01-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "spi.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */

#define TEST_DATA_SIZE 5U

SpiSettings spi_test_settings = {
  .baudrate = SPI_BAUDRATE_2_5MHZ,
  .mode = SPI_MODE_3,
  .sdo = { .port = GPIO_PORT_B, .pin = 15 },
  .sdi = { .port = GPIO_PORT_B, .pin = 14 },
  .sclk = { .port = GPIO_PORT_B, .pin = 13 },
  .cs = { .port = GPIO_PORT_B, .pin = 12 },
};

TASK(spi_api, TASK_STACK_1024) {
  spi_init(SPI_PORT_1, &spi_test_settings);
  spi_init(SPI_PORT_2, &spi_test_settings);

  uint8_t tx_data_1[TEST_DATA_SIZE] = { 0xAB, 0xAC, 0xAD, 0xAE, 0xAF };
  uint8_t rx_data_1[TEST_DATA_SIZE] = { 0x00 };

  uint8_t tx_data_2[TEST_DATA_SIZE] = { 0x10, 0x11, 0x12, 0x13, 0x14 };
  uint8_t rx_data_2[TEST_DATA_SIZE] = { 0x00 };

  while (true) {
    LOG_DEBUG("Exchanging data\n");
    spi_exchange(SPI_PORT_1, tx_data_1, sizeof(tx_data_1), rx_data_1, sizeof(rx_data_1));
    spi_exchange(SPI_PORT_2, tx_data_2, sizeof(tx_data_2), rx_data_2, sizeof(rx_data_2));

    LOG_DEBUG("------SPI PORT 1-----\n");
    for (size_t i = 0U; i < TEST_DATA_SIZE; i++) {
      LOG_DEBUG("Received byte %d: 0x%x\n", i, rx_data_1[i]);
      delay_ms(5);
    }

    delay_ms(500);

    LOG_DEBUG("------SPI PORT 2-----\n");
    for (size_t i = 0U; i < TEST_DATA_SIZE; i++) {
      LOG_DEBUG("Received byte %d: 0x%x\n", i, rx_data_2[i]);
      delay_ms(5);
    }

    delay_ms(500);
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

  tasks_init_task(spi_api, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
