/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for adbms1818
 *
 * @date   2025-05-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */
#include "adbms_afe.h"

TASK(adbms1818, TASK_STACK_1024) {
  AdbmsAfeStorage afe_storage;

  /* Based on spi smoke test (Not sure) */
  const SpiSettings spi_config = {
    .baudrate = SPI_BAUDRATE_2_5MHZ,
    .mode = SPI_MODE_3,
    .sdo = { .port = GPIO_PORT_B, .pin = 15 },
    .sdi = { .port = GPIO_PORT_B, .pin = 14 },
    .sclk = { .port = GPIO_PORT_B, .pin = 13 },
    .cs = { .port = GPIO_PORT_B, .pin = 12 },
  };

  AdbmsAfeSettings afe_settings = {
    .adc_mode = ADBMS_AFE_ADC_MODE_27KHZ,

    .cell_bitset[0] = 0x0FFFU,
    .aux_bitset[0] = 0x00FFU,

    .num_devices = 3,
    .num_cells = ADBMS_AFE_MAX_CELLS,
    .num_thermistors = ADBMS_AFE_MAX_THERMISTORS,

    .spi_settings = spi_config,
    .spi_port = SPI_PORT_2 /* Not sure */
  };

  StatusCode status = adbms_afe_init(&afe_storage, &afe_settings);
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("adbms_afe_init() failed: %d\n", status);
    while (true) {
    }
  }

  LOG_DEBUG("ADBMS1818 init OK – beginning conversions…\n");

  while (true) {
    status = adbms_afe_trigger_cell_conv(&afe_storage);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("ADCV (Trigger Cell Voltage Conversion) failed: %d\n", status);
      delay_ms(5);
      continue;
    }

    status = adbms_afe_read_cells(&afe_storage);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("RDCV (Read cell conversion) failed: %d\n", status);
      delay_ms(5);
      continue;
    }

    status = adbms_afe_trigger_aux_conv(&afe_storage, 4);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("ADAX (Trigger aux Voltage Conversion) failed: %d\n", status);
      delay_ms(5);
      continue;
    }

    status = adbms_afe_read_aux(&afe_storage, 4);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("RDAUX (Read aux conversion) failed: %d\n", status);
      delay_ms(5);
      continue;
    }

    for (size_t i = 0; i < ADBMS_AFE_MAX_CELLS; i++) {
      LOG_DEBUG("Cell %02u : %4u \n", (unsigned)i, (unsigned)(afe_storage.cell_voltages));
      delay_ms(5);
    }
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

  tasks_init_task(adbms1818, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
