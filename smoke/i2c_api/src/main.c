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
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "status.h"
#include "delay.h"
#include "i2c.h"

/* Intra-component Headers */

I2CSettings i2c_test_settings = {
  .speed = I2C_SPEED_STANDARD,
  .sda = {.port = GPIO_PORT_A, .pin = 10},
  .scl = {.port = GPIO_PORT_A, .pin = 9}
};

TASK(i2c_api, TASK_STACK_1024) {


  i2c_init(I2C_PORT_1, &i2c_test_settings);
  I2CAddress address = 100;
  uint8_t data[5] = {0x10, 0x11, 0x12, 0x13, 0x14};
  size_t length = 5;



  while (true) {
    i2c_write(I2C_PORT_1, address, data, length);
    delay_ms(500);
    
  }
}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(i2c_api, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}