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
#include "gpio_interrupts.h"
#include "interrupts.h"
#include "log.h"
#include "mcu.h"
#include "notify.h"
#include "status.h"
#include "tasks.h"

/* STM32 register defs for EXTI->SWIER1 */
#include "stm32l4xx.h"

/* Intra-component Headers */

/* Smoke Config */
#define NUM_EXTI_LINES_SMOKE 16U          // EXTI lines 0..15
#define EXTI_TIMEOUT_MS      100U
#define TEST_PORT            GPIO_PORT_C  // keep all on one port (driver indexes by pin)

uint32_t notification = 0;

static InterruptSettings s_rising_settings = {
  INTERRUPT_TYPE_INTERRUPT,
  INTERRUPT_PRIORITY_NORMAL,
  INTERRUPT_EDGE_RISING,
};

static StatusCode s_trigger_exti_swi(uint8_t line) {
  if (line >= 16U) {
    return STATUS_CODE_INVALID_ARGS;
  }

  EXTI->SWIER1 |= (1U << line);
  return STATUS_CODE_OK;
}

TASK(gpio_interrupts_api, TASK_STACK_1024) {
  uint8_t pass_count = 0;

  LOG_DEBUG("Starting GPIO interrupt smoke test...\r\n");
  delay_ms(10U);

  for (uint8_t line = 0; line < NUM_EXTI_LINES_SMOKE; ++line) {
    GpioAddress addr = { .port = TEST_PORT, .pin = line };

    LOG_DEBUG("Testing EXTI line %u (PC%u)...\r\n", line, line);
    delay_ms(10U);

    StatusCode status = gpio_it_init(&addr, &s_rising_settings, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("  FAIL: gpio_it_init returned %d\r\n", status);
      delay_ms(10U);
      continue;
    }

    status = gpio_register_interrupt(&addr, &s_rising_settings, (Event)line, gpio_interrupts_api);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("  FAIL: gpio_register_interrupt returned %d\r\n", status);
      delay_ms(10U);
      continue;
    }

    s_trigger_exti_swi(line);

    notification = 0;
    status = notify_wait(&notification, EXTI_TIMEOUT_MS);

    if (status == STATUS_CODE_OK && notify_check_event(&notification, (Event)line)) {
      LOG_DEBUG("PASS: EXTI line %u interrupt received\r\n", line);
      delay_ms(10U);
      pass_count++;
    } else {
      LOG_DEBUG("FAIL: EXTI line %u (status=%d, notif=0x%08lx)\r\n", line, status,
                (unsigned long)notification);
      delay_ms(10U);
    }
  }

  LOG_DEBUG("Results: %u/%u lines passed\r\n", pass_count, NUM_EXTI_LINES_SMOKE);
  delay_ms(10U);

  while (true) {
    delay_ms(1000U);
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
  interrupt_init();
  tasks_init();
  log_init();

  tasks_init_task(gpio_interrupts_api, TASK_PRIORITY(3), NULL);
  tasks_start();

  LOG_DEBUG("exiting main?\r\n");
  return 0;
}
