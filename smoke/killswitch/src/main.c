/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for killswitch API
 *
 * @brief  Tests GPIO initialization and interrupt setup for killswitch without calling
 *         killswitch.h directly. This allows testing the killswitch API pattern and behavior.
 *
 * @date   2026-01-07
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "gpio.h"
#include "gpio_interrupts.h"
#include "interrupts.h"
#include "log.h"
#include "mcu.h"
#include "notify.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */

/* Killswitch GPIO Configuration */
#define KILLSWITCH_EVENT 1U
#define KILLSWITCH_DEBOUNCE_MS 10U

static GpioAddress s_killswitch_address = { .port = GPIO_PORT_C, .pin = 5 };

static InterruptSettings s_killswitch_settings = {
  INTERRUPT_TYPE_INTERRUPT,
  INTERRUPT_PRIORITY_NORMAL,
  INTERRUPT_EDGE_RISING,
};

static StatusCode killswitch_api_test_init(Event event, const Task *task) {
  StatusCode status = STATUS_CODE_OK;

  /* Initialize GPIO pin as input with pull-down */
  LOG_DEBUG("Initializing killswitch GPIO pin\n");
  status = gpio_init_pin(&s_killswitch_address, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Error: Failed to initialize GPIO pin\n");
    return status;
  }

  /* Debounce delay */
  delay_ms(KILLSWITCH_DEBOUNCE_MS);

  /* Check current state */
  GpioState current_state = gpio_get_state(&s_killswitch_address);
  LOG_DEBUG("Killswitch current state: %s\n", (current_state == GPIO_STATE_LOW) ? "LOW" : "HIGH");

  /* Register interrupt if pin is in expected state */
  if (current_state == GPIO_STATE_LOW) {
    LOG_DEBUG("Registering interrupt handler\n");
    status = gpio_register_interrupt(&s_killswitch_address, &s_killswitch_settings, event, task);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Error: Failed to register interrupt\n");
      return status;
    }
    LOG_DEBUG("Interrupt registered successfully\n");
  } else {
    LOG_DEBUG("Warning: Killswitch pin is HIGH (may be already triggered)\n");
  }

  return status;
}

/* Interrupt handler task that receives killswitch events */
TASK(interrupt_handler, TASK_STACK_1024) {
  uint32_t notification = 0;

  while (true) {
    notify_get(&notification);
    if (notification & (1 << KILLSWITCH_EVENT)) {
      LOG_DEBUG("KILLSWITCH PRESSED\r\n");
    }
  }
}

TASK(killswitch, TASK_STACK_1024) {
  LOG_DEBUG("Killswitch smoke test task started\n");

  /* Initialize the killswitch API once */
  /* The interrupt_handler task will be notified when GPIO interrupt fires */
  StatusCode status = killswitch_api_test_init(KILLSWITCH_EVENT, (const Task *)&interrupt_handler);
  
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Killswitch initialization failed with status: %d\n", status);
    while (true) {
      delay_ms(1000U);
    }
  } else {
    LOG_DEBUG("Killswitch initialization successful\n");
    LOG_DEBUG("Waiting for GPIO interrupt on killswitch pin...\n");
  }

  /* Keep task alive - GPIO interrupt will notify the interrupt_handler task */
  while (true) {
    delay_ms(1000U);
    LOG_DEBUG("Killswitch smoke test running...\n");
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

  LOG_DEBUG("Starting killswitch API smoke test\n");

  tasks_init_task(interrupt_handler, TASK_PRIORITY(4), NULL);
  tasks_init_task(killswitch, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}