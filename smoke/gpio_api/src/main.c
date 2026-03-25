/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for gpio_api
 *
 * @date   2025-11-10
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "misc.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */

/* Configuration defines */
#define GPIO_TEST_MODE GPIO_INPUT_PULL_UP
#define GPIO_TEST_INIT_STATE GPIO_STATE_HIGH

/* All GPIO pins across all ports */
static const GpioAddress gpio_pins[] = {
  /* Port A pins */
  { .port = GPIO_PORT_A, .pin = 0U },
  { .port = GPIO_PORT_A, .pin = 1U },
  { .port = GPIO_PORT_A, .pin = 2U },
  { .port = GPIO_PORT_A, .pin = 3U },
  { .port = GPIO_PORT_A, .pin = 4U },
  { .port = GPIO_PORT_A, .pin = 5U },
  { .port = GPIO_PORT_A, .pin = 6U },
  { .port = GPIO_PORT_A, .pin = 7U },
  { .port = GPIO_PORT_A, .pin = 8U },
#ifndef STM32L4P5xx
  { .port = GPIO_PORT_A, .pin = 9U },
  { .port = GPIO_PORT_A, .pin = 10U },
#endif
  { .port = GPIO_PORT_A, .pin = 11U },
  { .port = GPIO_PORT_A, .pin = 12U },
  { .port = GPIO_PORT_A, .pin = 15U },

  /* Port B pins */
  { .port = GPIO_PORT_B, .pin = 0U },
  { .port = GPIO_PORT_B, .pin = 1U },
  { .port = GPIO_PORT_B, .pin = 2U },
  { .port = GPIO_PORT_B, .pin = 3U },
  { .port = GPIO_PORT_B, .pin = 4U },
  { .port = GPIO_PORT_B, .pin = 5U },
#ifndef STM32L433xx
  { .port = GPIO_PORT_B, .pin = 6U },
  { .port = GPIO_PORT_B, .pin = 7U },
#endif
  { .port = GPIO_PORT_B, .pin = 8U },
  { .port = GPIO_PORT_B, .pin = 9U },
  { .port = GPIO_PORT_B, .pin = 10U },
  { .port = GPIO_PORT_B, .pin = 11U },
  { .port = GPIO_PORT_B, .pin = 12U },
  { .port = GPIO_PORT_B, .pin = 13U },
  { .port = GPIO_PORT_B, .pin = 14U },
  { .port = GPIO_PORT_B, .pin = 15U },

  /* Port C pins */
  { .port = GPIO_PORT_C, .pin = 0U },
  { .port = GPIO_PORT_C, .pin = 1U },
  { .port = GPIO_PORT_C, .pin = 2U },
  { .port = GPIO_PORT_C, .pin = 3U },
  { .port = GPIO_PORT_C, .pin = 4U },
  { .port = GPIO_PORT_C, .pin = 5U },
  { .port = GPIO_PORT_C, .pin = 6U },
  { .port = GPIO_PORT_C, .pin = 7U },
  { .port = GPIO_PORT_C, .pin = 8U },
  { .port = GPIO_PORT_C, .pin = 9U },
#ifdef STM32L496xx
  { .port = GPIO_PORT_C, .pin = 10U },
  { .port = GPIO_PORT_C, .pin = 11U },
#endif
  { .port = GPIO_PORT_C, .pin = 12U },
  { .port = GPIO_PORT_C, .pin = 13U },
  { .port = GPIO_PORT_C, .pin = 14U },
  { .port = GPIO_PORT_C, .pin = 15U },

  /* Port D pins */
  { .port = GPIO_PORT_D, .pin = 0U },
  { .port = GPIO_PORT_D, .pin = 1U },
  { .port = GPIO_PORT_D, .pin = 2U },
  { .port = GPIO_PORT_D, .pin = 3U },
  { .port = GPIO_PORT_D, .pin = 4U },
  { .port = GPIO_PORT_D, .pin = 5U },
  { .port = GPIO_PORT_D, .pin = 6U },
  { .port = GPIO_PORT_D, .pin = 7U },
  { .port = GPIO_PORT_D, .pin = 8U },
  { .port = GPIO_PORT_D, .pin = 9U },
  { .port = GPIO_PORT_D, .pin = 10U },
  { .port = GPIO_PORT_D, .pin = 11U },
  { .port = GPIO_PORT_D, .pin = 12U },
  { .port = GPIO_PORT_D, .pin = 13U },
  { .port = GPIO_PORT_D, .pin = 14U },
  { .port = GPIO_PORT_D, .pin = 15U },

  /* Port E pins */
  { .port = GPIO_PORT_E, .pin = 0U },
  { .port = GPIO_PORT_E, .pin = 1U },
  { .port = GPIO_PORT_E, .pin = 2U },
  { .port = GPIO_PORT_E, .pin = 3U },
  { .port = GPIO_PORT_E, .pin = 4U },
  { .port = GPIO_PORT_E, .pin = 5U },
  { .port = GPIO_PORT_E, .pin = 6U },
  { .port = GPIO_PORT_E, .pin = 7U },
  { .port = GPIO_PORT_E, .pin = 8U },
  { .port = GPIO_PORT_E, .pin = 9U },
  { .port = GPIO_PORT_E, .pin = 10U },
  { .port = GPIO_PORT_E, .pin = 11U },
  { .port = GPIO_PORT_E, .pin = 12U },
  { .port = GPIO_PORT_E, .pin = 13U },
  { .port = GPIO_PORT_E, .pin = 14U },
  { .port = GPIO_PORT_E, .pin = 15U },
};

TASK(gpio_api, TASK_STACK_1024) {
  /* Initialize all GPIO pins */
  for (uint8_t i = 0U; i < SIZEOF_ARRAY(gpio_pins); i++) {
    StatusCode status = gpio_init_pin(&gpio_pins[i], GPIO_TEST_MODE, GPIO_TEST_INIT_STATE);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Fail\r\n");
    }
  }

  while (true) {
    LOG_DEBUG("\n\r\\\\------------------------ GPIO State ------------------------//\n\r");
    delay_ms(10U);

    for (uint8_t i = 0U; i < SIZEOF_ARRAY(gpio_pins); i++) {
      GpioState state = gpio_get_state(&gpio_pins[i]);
      LOG_DEBUG("Pin %c%d: %s\r\n", (char)(gpio_pins[i].port + 'A'), gpio_pins[i].pin, (state == GPIO_STATE_HIGH) ? "HIGH" : "LOW ");
      delay_ms(10U);
    }

    delay_ms(1000U);
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

  tasks_init_task(gpio_api, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
