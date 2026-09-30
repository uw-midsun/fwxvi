/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for button_led_pin
 *
 * @date   2026-09-24
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
#include "stm32l4xx_hal.h"

#define HIGH_TICK_0 25U
#define HIGH_TICK_1 70U

/* Intra-component Headers */

static inline void send_bit(GPIO_TypeDef *port, uint16_t pin, uint8_t bit) {
  uint32_t high_ticks;

  if (bit == 0U) {
    high_ticks = HIGH_TICK_0;
  } else {
    high_ticks = HIGH_TICK_1;
  }

  uint32_t start = DWT->CYCCNT;

  port->BSRR = pin; 
  while ((uint32_t)(DWT->CYCCNT - start) < high_ticks) {
  }

  port->BSRR = (uint32_t)pin << 16U;
  while ((uint32_t)(DWT->CYCCNT - start) < 100U) {
  }
}

static inline void reset_led(void){
  uint32_t start = DWT->CYCCNT;
  while ((uint32_t)(DWT->CYCCNT - start) < 8000U) {
  }
}

TASK(button_led_pin, TASK_STACK_1024) {
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0U;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  GPIO_InitTypeDef gpio = { 0 };
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Speed =  GPIO_SPEED_FREQ_VERY_HIGH;
  gpio.Pull = GPIO_NOPULL;
  gpio.Pin = GPIO_PIN_7;
  HAL_GPIO_Init(GPIOB, &gpio);

  gpio.Pin = GPIO_PIN_5; 
  HAL_GPIO_Init(GPIOA, &gpio);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7,GPIO_PIN_RESET);

  taskENTER_CRITICAL();

  while (true) {

    send_bit(GPIOB, GPIO_PIN_7,1U); 
    send_bit(GPIOB, GPIO_PIN_7,1U); 
    send_bit(GPIOB, GPIO_PIN_7,1U); 
    send_bit(GPIOB, GPIO_PIN_7,1U); 
    send_bit(GPIOB, GPIO_PIN_7,1U); 
    send_bit(GPIOB, GPIO_PIN_7,1U); 
    send_bit(GPIOB, GPIO_PIN_7,1U); 
    send_bit(GPIOB, GPIO_PIN_7,1U); 

    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 

    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 

    send_bit(GPIOB, GPIO_PIN_7,1U); 
    send_bit(GPIOB, GPIO_PIN_7,1U); 
    send_bit(GPIOB, GPIO_PIN_7,1U); 
    send_bit(GPIOB, GPIO_PIN_7,1U); 
    send_bit(GPIOB, GPIO_PIN_7,1U); 
    send_bit(GPIOB, GPIO_PIN_7,1U); 
    send_bit(GPIOB, GPIO_PIN_7,1U); 
    send_bit(GPIOB, GPIO_PIN_7,1U); 

    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 

    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 

    send_bit(GPIOB, GPIO_PIN_7,1U); 
    send_bit(GPIOB, GPIO_PIN_7,1U); 
    send_bit(GPIOB, GPIO_PIN_7,1U); 
    send_bit(GPIOB, GPIO_PIN_7,1U); 
    send_bit(GPIOB, GPIO_PIN_7,1U); 
    send_bit(GPIOB, GPIO_PIN_7,1U); 
    send_bit(GPIOB, GPIO_PIN_7,1U); 
    send_bit(GPIOB, GPIO_PIN_7,1U); 

    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 

    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 
    send_bit(GPIOB, GPIO_PIN_7,0U); 

    reset_led();

  }

  taskEXIT_CRITICAL();

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
  gpio_init();
  
  tasks_init_task(button_led_pin, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}