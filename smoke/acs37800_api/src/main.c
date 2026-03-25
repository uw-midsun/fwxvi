/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for acs37800_smoke
 *
 * @date   2025-09-21
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

#include "current_acs37800.h"
#include "current_acs37800_defs.h"
#include "delay.h"
#include "gpio.h"
#include "i2c.h"
#include "log.h"
#include "mcu.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */

static void print_register_bytes(const char *label, uint32_t raw_data) {
  uint8_t bytes[4] = {
    (uint8_t)(raw_data & 0xFF),
    (uint8_t)((raw_data >> 8) & 0xFF),
    (uint8_t)((raw_data >> 16) & 0xFF),
    (uint8_t)((raw_data >> 24) & 0xFF),
  };

  LOG_DEBUG("%s:\n", label);
  for (size_t i = 0; i < 4; i++) {
    LOG_DEBUG("Byte[%u] = 0x%02X\r\n", i, bytes[i]);
    delay_ms(10);
  }
}

I2CSettings i2c_settings = { .speed = I2C_SPEED_STANDARD, .sda = { .port = GPIO_PORT_B, .pin = 11 }, .scl = { .port = GPIO_PORT_B, .pin = 10 } };

#define I2CP I2C_PORT_2
#define I2CA 0x61

ACS37800Storage storage;

TASK(acs37800_smoke, TASK_STACK_1024) {
  i2c_init(I2CP, &i2c_settings);
  acs37800_init(&storage, I2CP, I2CA);

  while (true) {
    LOG_DEBUG("---- ACS37800 SMOKE TEST ----\r\r\n\n");
    delay_ms(10);

    // To find the I2C address of the board
    // uint8_t rx_buf[4];
    // bool found = false;
    // for (I2CAddress addr = 0x08; addr <= 0x78; addr++) {
    //   StatusCode status = i2c_read_mem(I2CP, addr, ACS37800_REG_STATUS, rx_buf, sizeof(rx_buf));
    //   if (status == STATUS_CODE_OK) {
    //     LOG_DEBUG("Found device at address 0x%02X\r\n", addr);
    //     delay_ms(50);
    //     found = true;
    //   }
    // }
    // if (!found) {
    //   LOG_DEBUG("No device found on bus\r\n");
    // }

#ifdef MS_PLATFORM_X86
    acs37800_set_current(3.5f);
    acs37800_set_voltage(30.0f);
#endif
    float current = 0.0f;

    if (acs37800_get_current(&storage, &current) != STATUS_CODE_OK) {
      LOG_DEBUG("ERROR GETTING CURRENT\r\n");
      delay_ms(10);
      continue;
    }

    int32_t current_A = (int32_t)(current * 1000.0f);

    uint32_t whole = (uint32_t)(current_A < 0 ? -(current_A / 1000) : (current_A / 1000));
    uint32_t frac = (uint32_t)(current_A < 0 ? -(current_A % 1000) : (current_A % 1000));

    LOG_DEBUG("CURRENT_DATA: %s%u.%03u A\r\n", current_A < 0 ? "-" : "", (unsigned)whole, (unsigned)frac);

    delay_ms(20);

    float voltage = 0.0f;

    if (acs37800_get_voltage(&storage, &voltage) != STATUS_CODE_OK) {
      LOG_DEBUG("ERROR GETTING VOLTAGE\r\n");
      delay_ms(10);
      continue;
    }

    int32_t voltage_mV = (int32_t)(voltage * 1000.0f);

    whole = (uint32_t)(voltage_mV < 0 ? -(voltage_mV / 1000) : (voltage_mV / 1000));
    frac = (uint32_t)(voltage_mV < 0 ? -(voltage_mV % 1000) : (voltage_mV % 1000));

    LOG_DEBUG("VOLTAGE_DATA: %s%u.%03u mV\r\n", voltage_mV < 0 ? "-" : "", (unsigned)whole, (unsigned)frac);
    delay_ms(20);

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
  printf("Starting smoke test...\r\n");

  tasks_init_task(acs37800_smoke, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
