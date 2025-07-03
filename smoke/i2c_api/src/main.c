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

#ifdef MS_PLATFORM_X86
StatusCode i2c_set_data(I2CPort i2c, uint8_t *tx_data, size_t tx_len);
#endif


/* Intra-component Headers */

I2CSettings i2c_test_settings = { .speed = I2C_SPEED_STANDARD, .sda = { .port = GPIO_PORT_B, .pin = 11 }, .scl = { .port = GPIO_PORT_B, .pin = 10 } };

TASK(i2c_api, TASK_STACK_1024) {
  i2c_init(I2C_PORT_2, &i2c_test_settings);
  I2CAddress address = 0x0;
  uint8_t data[5] = { 0x15, 0x16, 0x17, 0x18, 0x19 };
  uint8_t read_data[5];
  uint8_t read_data2[5];

  size_t length = 5;

  while (true) {
    LOG_DEBUG("WRITE\n");
    i2c_write(I2C_PORT_2, address, data, length);
    delay_ms(500);

    LOG_DEBUG("WRITE REG\n");
    i2c_write_reg(I2C_PORT_2, address, 0x12, data, length);
    delay_ms(500);

    i2c_set_data(I2C_PORT_2, data, length);
    LOG_DEBUG("READ\n");
    i2c_read(I2C_PORT_2, address, read_data, length);
    delay_ms(500);

    i2c_set_data(I2C_PORT_2, data, length);
    LOG_DEBUG("READ REG\n");
    i2c_read_reg(I2C_PORT_2, address, 0x12, read_data2, length);
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

  tasks_init_task(i2c_api, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
