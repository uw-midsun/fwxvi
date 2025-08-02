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
#ifdef MS_PLATFORM_X86
  s_settings.adc_mode = ADBMS_AFE_ADC_MODE_27KHZ;
  s_settings.adc_mode = ADBMS_AFE_ADC_MODE_27KHZ;

  for (size_t i = 0; i < ADBMS_AFE_MAX_DEVICES; ++i) {
    s_settings.cell_bitset[i] = 0xFFFF;
    s_settings.aux_bitset[i] = 0xFFFF;
  }

  s_settings.num_devices = 3;
  s_settings.num_cells = ADBMS_AFE_MAX_CELLS;
  s_settings.num_thermistors = ADBMS_AFE_MAX_THERMISTORS;
#else
  AdbmsAfeStorage s_afe;
  AdbmsAfeSettings s_settings;

  /* Based on spi smoke test (Not sure) */
  const SpiSettings spi_config = {
    .baudrate = SPI_BAUDRATE_2_5MHZ,
    .mode = SPI_MODE_3,
    .sdo = { .port = GPIO_PORT_B, .pin = 15 },
    .sdi = { .port = GPIO_PORT_B, .pin = 14 },
    .sclk = { .port = GPIO_PORT_B, .pin = 13 },
    .cs = { .port = GPIO_PORT_B, .pin = 12 },
  };

  AdbmsAfeSettings s_settings = {
    .adc_mode = ADBMS_AFE_ADC_MODE_27KHZ,

    .cell_bitset[0] = 0x0FFFU,
    .aux_bitset[0] = 0x00FFU,

    .num_devices = 3,
    .num_cells = ADBMS_AFE_MAX_CELLS,
    .num_thermistors = ADBMS_AFE_MAX_THERMISTORS,

    .spi_settings = spi_config,
    .spi_port = SPI_PORT_2 /* Not sure */
  };
#endif

  StatusCode status = adbms_afe_init(&s_afe, &s_settings);
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("adbms_afe_init() failed: %d\n", status);
  }

#ifdef MS_PLATFORM_x86
  for (size_t i = 0; i < ADBMS_AFE_MAX_DEVICES; ++i) {
    s_afe.device_configs->devices[i].cfgA.discharge_bitset = (1u << 12) - 1;
    s_afe.device_configs->devices[i].cfgB.discharge_bitset = (1u << 7) - 1;
  }
#endif

  while (true) {
    LOG_DEBUG("Numbers of cells is: %u\n", s_afe.settings->num_cells);
    status = adbms_afe_trigger_cell_conv(&s_afe);
    if (status != STATUS_CODE_OK) {
      // LOG_DEBUG("ADCV (Trigger Cell Voltage Conversion) failed: %d\n", status);
      delay_ms(5);
      LOG_DEBUG("Coninue");
    }

    status = adbms_afe_read_cells(&s_afe);
    if (status != STATUS_CODE_OK) {
      // LOG_DEBUG("RDCV (Read cell conversion) failed: %d\n", status);
      delay_ms(5);
      LOG_DEBUG("Coninue");
    }

    status = adbms_afe_trigger_aux_conv(&s_afe, 4);
    if (status != STATUS_CODE_OK) {
      // LOG_DEBUG("ADAX (Trigger aux Voltage Conversion) failed: %d\n", status);
      delay_ms(5);
      LOG_DEBUG("Coninue");
    }

    status = adbms_afe_read_aux(&s_afe, 4);
    if (status != STATUS_CODE_OK) {
      // LOG_DEBUG("RDAUX (Read aux conversion) failed: %d\n", status);
      delay_ms(5);
      LOG_DEBUG("Coninue");
    }

    for (size_t i = 0; i < ADBMS_AFE_MAX_CELLS; i++) {
      //  LOG_DEBUG("Cell %02u : %4u mV\n",
      //           (unsigned)i,
      //           (unsigned)s_afe.cell_voltages[i]);
      delay_ms(30);
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
  log_init();
  tasks_init();

  tasks_init_task(adbms1818, TASK_PRIORITY(3U), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
