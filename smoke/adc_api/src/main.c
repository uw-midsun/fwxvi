/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for adc_api
 *
 * @date   2025-02-04
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
#include "adc.h"
#include "misc.h"

/* Intra-component Headers */

/* Channels 1-4 are occupied by pins C0-C3 */
/* Channels 5-12 are occupied by pins A0-A7 */
/* Channels 13-14 are occupied by pins C4-C5 */
/* Channels 15-16 are occupied by pins B0-B1 */
static const GpioAddress adc_address[] = {
  { .port = GPIO_PORT_A, .pin = 0U }, { .port = GPIO_PORT_A, .pin = 1U },
  { .port = GPIO_PORT_A, .pin = 2U }, { .port = GPIO_PORT_A, .pin = 3U },
  { .port = GPIO_PORT_A, .pin = 4U }, { .port = GPIO_PORT_A, .pin = 5U },
  { .port = GPIO_PORT_A, .pin = 6U }, { .port = GPIO_PORT_A, .pin = 7U },
  { .port = GPIO_PORT_B, .pin = 0U }, { .port = GPIO_PORT_B, .pin = 1U },
  { .port = GPIO_PORT_C, .pin = 0U }, { .port = GPIO_PORT_C, .pin = 1U },
  { .port = GPIO_PORT_C, .pin = 2U }, { .port = GPIO_PORT_C, .pin = 3U },
  { .port = GPIO_PORT_C, .pin = 4U }, { .port = GPIO_PORT_C, .pin = 5U },
};

TASK(adc_api, TASK_STACK_1024) {
  for (uint8_t i = 0U; i < SIZEOF_ARRAY(adc_address); i++) {
    gpio_init_pin(&adc_address[i], GPIO_ANALOG, GPIO_STATE_LOW);
    adc_add_channel(&adc_address[i]);
  }

  adc_init();
  uint16_t data = 0U;

  while (true) {
    adc_run();
  
    printf("\n\r\\\\------------------------ ADC Data ------------------------//\n\r");
    for (uint8_t i = 0U; i < SIZEOF_ARRAY(adc_address); i++) {
      adc_read_converted(&adc_address[i], &data);
      printf("Pin %c%d: %d\t", (char)(adc_address[i].port + 'A'), adc_address[i].pin, data);

      if ((i + 1U) % 4U == 0U) {
        printf("\n\r");
      }
    }

    delay_ms(1000U);
  }

}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(adc_api, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}