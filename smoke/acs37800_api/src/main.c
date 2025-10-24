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

static void print_bytes(const char *label, const uint8_t *data, size_t len) {
  LOG_DEBUG("%s:\n", label);
  for (size_t i = 0; i < len; i++) {
    LOG_DEBUG("Byte[%zu] = 0x%02X\n", i, data[i]);
  }
}

I2CSettings i2c_settings = { .speed = I2C_SPEED_STANDARD, .sda = { .port = GPIO_PORT_B, .pin = 11 }, .scl = { .port = GPIO_PORT_B, .pin = 10 } };

#define I2CP I2C_PORT_1
#define I2CA 0x0

ACS37800Storage storage;

void build_packet(uint8_t packet[4], int16_t voltage, int16_t current) {
  packet[0] = (uint8_t)(voltage >> 8);
  packet[1] = (uint8_t)(voltage & 0xFF);
  packet[2] = (uint8_t)(current >> 8);
  packet[3] = (uint8_t)(current & 0xFF);
}

TASK(acs37800_smoke, TASK_STACK_1024) {
  i2c_init(I2CP, &i2c_settings);
  acs37800_init(&storage, I2CP, I2CA);

  uint8_t packet[4];
  uint32_t rx_packet;
  while (true) {
    LOG_DEBUG("--- ACS37800 SMOKE TEST ---\n\n");
    build_packet(packet, 12000, 13000);
    print_bytes("PACKET: ", (uint8_t *)(&packet), 4);
#ifdef MS_PLATFORM_X86
    i2c_set_rx_data(I2CP, (uint8_t *)(&packet), 4);
#endif
    float current;
    acs37800_get_current(&storage, &current);

    LOG_DEBUG("CURRENT_DATA: %f\n", current);

#ifdef MS_PLATFORM_X86
    i2c_set_rx_data(I2CP, (uint8_t *)(&packet), 4);
#endif

    float voltage;

    acs37800_get_voltage(&storage, &voltage);
    LOG_DEBUG("VOLTAGE DATA: %f\n", voltage);

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

  tasks_init_task(acs37800_smoke, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
