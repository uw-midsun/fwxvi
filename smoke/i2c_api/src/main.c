/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for i2c_api
 *
 * @date   2025-01-14
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "gpio.h"
#include "i2c.h"
#include "log.h"
#include "mcu.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */

I2CSettings i2c_test_settings = { .speed = I2C_SPEED_STANDARD, .sda = { .port = GPIO_PORT_B, .pin = 11 }, .scl = { .port = GPIO_PORT_B, .pin = 10 } };

static void print_bytes(const char *label, const uint8_t *data, size_t len) {
  LOG_DEBUG("%s:\n", label);
  for (size_t i = 0; i < len; i++) {
    LOG_DEBUG("  Byte[%zu] = 0x%02X\n", i, data[i]);
  }
}

TASK(i2c_api, TASK_STACK_1024) {
  i2c_init(I2C_PORT_2, &i2c_test_settings);
  I2CAddress address = 0x0;
  size_t length = 5;

  uint8_t data[5] = { 0x15, 0x16, 0x17, 0x18, 0x19 };
  uint8_t read_data[5] = { 0 };
  uint8_t read_data2[5] = { 0 };

  uint8_t rx_sim_data[5] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xAA };
  uint8_t tx_sim_data[6] = { 0 };

  while (true) {
    LOG_DEBUG("-------- I2C SMOKE TEST --------\n\n");

    LOG_DEBUG("-------- Writing to device --------\n");
    i2c_write(I2C_PORT_2, address, data, length);
    delay_ms(500);

#ifdef MS_PLATFORM_X86
    i2c_get_tx_data(I2C_PORT_2, tx_sim_data, length);
    print_bytes("Get TX data from simulated write buffer", tx_sim_data, length);
    LOG_DEBUG("\n");
#endif

    LOG_DEBUG("-------- Writing to register 0x12 --------\n");
    i2c_write_reg(I2C_PORT_2, address, 0x12, data, length);
    delay_ms(500);

#ifdef MS_PLATFORM_X86
    i2c_get_tx_data(I2C_PORT_2, tx_sim_data, length + 1);
    print_bytes("Get TX data from simulated write buffer", tx_sim_data, length + 1);

    i2c_set_rx_data(I2C_PORT_2, rx_sim_data, length);
#endif

    LOG_DEBUG("-------- Reading from device --------\n");
    i2c_read(I2C_PORT_2, address, read_data, length);
    print_bytes("READ", read_data, length);
    LOG_DEBUG("\n");
    delay_ms(500);

#ifdef MS_PLATFORM_X86
    i2c_set_rx_data(I2C_PORT_2, rx_sim_data, length);
#endif

    LOG_DEBUG("-------- Reading from register 0x12 --------\n");
    i2c_read_reg(I2C_PORT_2, address, 0x12, read_data2, length);
    print_bytes("READ REG", read_data2, length);
    LOG_DEBUG("\n");

#ifdef MS_PLATFORM_X86
    /* Clear the register write */
    i2c_get_tx_data(I2C_PORT_2, tx_sim_data, 1);
#endif

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
  tasks_init();
  log_init();

  tasks_init_task(i2c_api, TASK_PRIORITY(3), NULL);
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
