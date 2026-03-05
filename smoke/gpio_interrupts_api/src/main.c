/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for gpio_interrupts_api
 *
 * @date   2025-07-19
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
#include "gpio_interrupts.h"
#include "notify.h"

/* STM32 register defs for EXTI->SWIER1 */
#include "stm32l4xx.h"

/* Intra-component Headers */

/* Killswitch GPIO Configuration */
#define KILLSWITCH_EVENT 1U
#define KILLSWITCH_DEBOUNCE_MS 10U

/* Smoke Config */
#define NUM_EXTI_LINES_SMOKE 16U          // EXTI lines 0..15
#define EXTI_TIMEOUT_MS      100U
#define DEBOUNCE_MS          10U
#define TEST_PORT            GPIO_PORT_C  // keep all on one port (driver indexes by pin)

static GpioAddress s_killswitch_address = { .port = GPIO_PORT_C, .pin = 5 };
uint32_t notification = 0;

static InterruptSettings s_killswitch_settings = {
  INTERRUPT_TYPE_INTERRUPT,
  INTERRUPT_PRIORITY_NORMAL,
  INTERRUPT_EDGE_FALLING,
};

static StatusCode s_trigger_exti_swi(uint8_t line) {
  if (line >= 16U ) {
    return STATUS_CODE_INVALID_ARGS;
  }

  EXTI->SWIER1 |= (1U << line);
  return STATUS_CODE_OK;
}

TASK(gpio_interrupts_api, TASK_STACK_1024) {

  StatusCode status = STATUS_CODE_OK;
  /* Initialize GPIO pin as input with pull-down */
  LOG_DEBUG("Initializing killswitch GPIO interrupt...\r\n");
  delay_ms(10U);
  /* configure gpio for interrupt mode*/

  for (uint8_t line = 0; line < NUM_EXTI_LINES_SMOKE; ++line)
  status = gpio_it_init(&s_killswitch_address, &s_killswitch_settings, GPIO_INPUT_PULL_UP, GPIO_STATE_HIGH);
  if (status != STATUS_CODE_OK) { 
    LOG_DEBUG("gpio_it_init failed: %d\r\n", status);
    while(true) {
      delay_ms(1000U);
    }
  }
  status = gpio_register_interrupt(&s_killswitch_address, &s_killswitch_settings, KILLSWITCH_EVENT, gpio_interrupts_api);
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("gpio_register_interrupt failed: %d\r\n", status);
    while(true) {
      delay_ms(1000U);
    }
  }

  // software trigger
  status = gpio_trigger_interrupt(&s_killswitch_address);
  LOG_DEBUG("gpio_trigger_interrupt returned: %d\r\n", status);

  while (true) {
    notification = 0;

    status = notify_wait(&notification, 100U);

    if (status == STATUS_CODE_OK) {
      if (notify_check_event(&notification, KILLSWITCH_EVENT)) {
        LOG_DEBUG("Pass\r\n");
        delay_ms(KILLSWITCH_DEBOUNCE_MS);
      } else {
        LOG_DEBUG("got notif: 0x%08lx\r\n", (unsigned long)notification);
      }
    }
  }
}

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
int main(int argc, char *argv[]) {
  mpxe_init(argc, argv);
#else
int main(void) {
#endif
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(gpio_interrupts_api, TASK_PRIORITY(3), NULL);
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
