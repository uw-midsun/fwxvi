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

// port + pin

/* Intra-component Headers */

/* Killswitch GPIO Configuration */
#define KILLSWITCH_EVENT 1U
#define KILLSWITCH_DEBOUNCE_MS 10U

static GpioAddress s_killswitch_address = { .port = GPIO_PORT_C, .pin = 5 };
uint32_t notification = 0;

static InterruptSettings s_killswitch_settings = {
  INTERRUPT_TYPE_INTERRUPT,
  INTERRUPT_PRIORITY_NORMAL,
  INTERRUPT_EDGE_FALLING,
};

TASK(gpio_interrupts_api, TASK_STACK_1024) {

  StatusCode status = STATUS_CODE_OK;
  /* Initialize GPIO pin as input with pull-down */
  LOG_DEBUG("Initializing killswitch GPIO interrupt...\r\n");
  delay_ms(10U);
  /* configure gpio for interrupt mode*/
  status = gpio_it_init(&s_killswitch_address, &s_killswitch_settings, GPIO_INPUT_PULL_UP, GPIO_STATE_HIGH);
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("gpio_it_init failed: %d\r\n", status);
    // debugging error message
    // LOG_DEBUG("addr port=%d pin=%d\r\n", s_killswitch_address.port, s_killswitch_address.pin);
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
