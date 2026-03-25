/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for adbms1818_driver
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
#include "adbms_afe_crc15.h"
#include "adbms_afe_regs.h"
#include "thermistor.h"

#define ADBMS1818_AFE_THERMISTOR_VOLTAGE_LOOKUP(s_afe, board, thermistor_num) (s_afe.thermistor_voltages[(board * ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE) + thermistor_num])

#define ADBMS1818_AFE_CELL_VOLTAGE_LOOKUP(s_afe, cell_raw_indx) (s_afe.cell_voltages[s_afe.cell_result_lookup[cell_raw_indx]])

const SpiSettings spi_config = {
  .baudrate = SPI_BAUDRATE_312_5KHZ,
  .mode = SPI_MODE_3,
  .sdo = { .port = GPIO_PORT_B, .pin = 15 },
  .sdi = { .port = GPIO_PORT_B, .pin = 14 },
  .sclk = { .port = GPIO_PORT_B, .pin = 13 },
  .cs = { .port = GPIO_PORT_B, .pin = 12 },
};

static AdbmsAfeSettings s_settings = {
  .adc_mode = ADBMS_AFE_ADC_MODE_7KHZ, .num_devices = 1, .num_cells = ADBMS_AFE_MAX_CELLS_PER_DEVICE, .num_thermistors = 1, .spi_settings = &spi_config, .spi_port = SPI_PORT_2
};

static AdbmsAfeStorage s_afe;

void wakeup() {
  gpio_set_state(&spi_config.cs, GPIO_STATE_LOW);
  gpio_set_state(&spi_config.cs, GPIO_STATE_HIGH);
  delay_ms(1);
}

void build_cmd(uint16_t cmd_val, uint8_t *buf) {
  buf[0] = (uint8_t)(cmd_val >> 8);
  buf[1] = (uint8_t)(cmd_val & 0xFF);
  uint16_t crc = crc15_calculate(buf, 2);
  buf[2] = (uint8_t)(crc >> 8);
  buf[3] = (uint8_t)(crc & 0xFF);
}

TASK(adbms1818_driver, TASK_STACK_1024) {
  StatusCode status = adbms_afe_init(&s_afe, &s_settings);

  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("adbms_afe_init() failed: %d\n", status);
  }

  delay_ms(10);

#ifdef MS_PLATFORM_x86
  for (size_t i = 0; i < ADBMS_AFE_MAX_DEVICES; ++i) {
    s_afe.device_configs->devices[i].cfgA.discharge_bitset = (1u << 12) - 1;
    s_afe.device_configs->devices[i].cfgB.discharge_bitset = (1u << 7) - 1;
  }
#endif

  while (true) {
    status = adbms_afe_trigger_cell_conv(&s_afe);

    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("ADCV (Trigger Cell Voltage Conversion) failed: %d\n", status);
    }

    delay_ms(10);

    status = adbms_afe_read_cells(&s_afe);

    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("RDCV (Read cell conversion) failed: %d\n", status);
      delay_ms(5);
      LOG_DEBUG("Coninue");
    }

    for (size_t i = 0; i < s_afe.settings->num_cells; i++) {
      LOG_DEBUG("Cell %02u: %4u 100uV\n", (unsigned)i, ADBMS1818_AFE_CELL_VOLTAGE_LOOKUP(s_afe, i));
      delay_ms(30);
    }

    adbms_afe_toggle_cell_discharge(&s_afe, 1, true);
    adbms_afe_write_config(&s_afe);

    uint8_t rx_cfgA[8] = { 0 };
    uint8_t rx_cfgB[8] = { 0 };
    uint8_t cmd[4] = { 0 };

    /* --- Read back CFG A --- */
    wakeup();
    build_cmd(ADBMS1818_RDCFGA_RESERVED, cmd);
    spi_exchange(SPI_PORT_2, cmd, 4, rx_cfgA, 8);
    delay_ms(10);

    LOG_DEBUG("CFG A Read:  %02X %02X %02X %02X %02X %02X [PEC: %02X %02X]\r\n", rx_cfgA[0], rx_cfgA[1], rx_cfgA[2], rx_cfgA[3], rx_cfgA[4], rx_cfgA[5], rx_cfgA[6], rx_cfgA[7]);

    /* --- Read back CFG B --- */
    wakeup();
    build_cmd(ADBMS1818_RDCFGB_RESERVED, cmd);
    spi_exchange(SPI_PORT_2, cmd, 4, rx_cfgB, 8);
    delay_ms(10);
    LOG_DEBUG("CFG B Read:  %02X %02X %02X %02X %02X %02X [PEC: %02X %02X]\r\n", rx_cfgB[0], rx_cfgB[1], rx_cfgB[2], rx_cfgB[3], rx_cfgB[4], rx_cfgB[5], rx_cfgB[6], rx_cfgB[7]);

    for (uint8_t i = 0U; i < s_afe.settings->num_devices; i++) {
      for (uint8_t j = 0U; j < s_afe.settings->num_thermistors; j++) {
        status = adbms_afe_trigger_thermistor_conv(&s_afe, i, j);

        if (status != STATUS_CODE_OK) {
          LOG_DEBUG("adbms_afe_trigger_thermistor_conv() failed: %d\r\n", status);
        }

        delay_ms(10);

        status = adbms_afe_read_thermistor(&s_afe, i, j);

        if (status != STATUS_CODE_OK) {
          LOG_DEBUG("adbms_afe_read_thermistor() failed: %d\r\n", status);
        } else {
          uint16_t temperature_c = calculate_board_thermistor_temperature(ADBMS1818_AFE_THERMISTOR_VOLTAGE_LOOKUP(s_afe, i, j) / 10U);
          LOG_DEBUG("Board number: %u, Thermistor: %u has voltage: %u 100uV, and temperature: %u C\r\n", i, j, ADBMS1818_AFE_THERMISTOR_VOLTAGE_LOOKUP(s_afe, i, j), temperature_c);
        }
      }
    }

    for (uint8_t i = 0U; i < s_afe.settings->num_devices; i++) {
      status = adbms_afe_trigger_board_temp_conv(&s_afe, i);

      if (status != STATUS_CODE_OK) {
        LOG_DEBUG("adbms_afe_trigger_board_temp_conv() failed: %d\r\n", status);
      }

      delay_ms(10);
    }

    for (uint8_t i = 0U; i < s_afe.settings->num_devices; i++) {
      status = adbms_afe_read_board_temp(&s_afe, i);

      if (status != STATUS_CODE_OK) {
        LOG_DEBUG("adbms_afe_read_board_temp() failed: %d\n", status);
      } else {
        uint16_t temperature_c = calculate_board_thermistor_temperature(s_afe.board_thermistor_voltages[i] / 10U);
        LOG_DEBUG("Board number: %u has Board thermistor voltage: %u 100uV, and temperature: %u C\r\n", i, s_afe.board_thermistor_voltages[i], temperature_c);
      }
      adbms_afe_write_config(&s_afe);

      uint8_t rx_cfgA[8] = { 0 };
      uint8_t rx_cfgB[8] = { 0 };
      uint8_t cmd[4] = { 0 };

      /* --- Read back CFG A --- */
      wakeup();
      build_cmd(ADBMS1818_RDCFGA_RESERVED, cmd);
      spi_exchange(SPI_PORT_2, cmd, 4, rx_cfgA, 8);
      delay_ms(10);
    }

    delay_ms(1000);
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

  tasks_init_task(adbms1818_driver, TASK_PRIORITY(3U), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
