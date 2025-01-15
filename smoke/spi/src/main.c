/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for [PROJECT NAME]
 *
 * @date   [YYYY/MM/DD]
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "gpio.h"
#include "spi.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */

GpioAddress pa0_led = { .pin = 0U, .port = GPIO_PORT_A };



TASK(SpiTest, TASK_STACK_512) {
  uint8_t tx_data=0xff;
  uint8_t rx_data=0x00;
  SpiSettings spi_settings{;
    .baudrate=10e6,
    .mode=SPI_MODE_0,
    .sdo = { .pin = 5, .port = GPIO_PORT_B },
    .sdi = { .pin = 6, .port = GPIO_PORT_B }, 
    .sclk = { .pin = 7, .port = GPIO_PORT_B },
    .cs = { .pin = 8, .port = GPIO_PORT_B },
  };
  gpio_init_pin(&pa0_led, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  while (true) {
    LOG_DEBUG("Sending data via SPI: 0x%X\n", tx_data);
    status_ok_or_return(spi_transmit(SPI_PORT_1, &tx_data, sizeof(tx_data)));
    status_ok_or_return(spi_receive(SPI_PORT_1, &rx_data, sizeof(rx_data)));
    LOG_DEBUG("Received data via SPI: 0x%X\n", rx_data);
    if (tx_data == rx_data) {
      LOG_DEBUG("SPI communication successful: 0x%X == 0x%X\n", tx_data, rx_data);
      gpio_toggle_state(&pa0_led);  // Toggle LED to indicate success
    } else {
      LOG_DEBUG("SPI communication failed: 0x%X != 0x%X\n", tx_data, rx_data);
    }
    delay_ms(1000);
  }
}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(SpiTest, TASK_PRIORITY(3UL), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
