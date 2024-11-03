/************************************************************************************************
 * gpio.c
 *
 * GPIO Library Source Code
 *
 * Created: 2024-11-02
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "stm32l433xx.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_rcc.h"
#include "task.h"

/* Intra-component Headers */
#include "gpio.h"
#include "status.h"

static const uint32_t s_gpio_mode_map[] = {
  [GPIO_ANALOG] = GPIO_MODE_ANALOG,
  [GPIO_INPUT_FLOATING] = GPIO_MODE_INPUT,
  [GPIO_OUTPUT_OPEN_DRAIN] = GPIO_MODE_OUTPUT_OD,
  [GPIO_OUTPUT_PUSH_PULL] = GPIO_MODE_OUTPUT_PP,
  [GPIO_ALFTN_OPEN_DRAIN] = GPIO_MODE_AF_OD,
  [GPIO_ALTFN_PUSH_PULL] = GPIO_MODE_AF_PP,
};

StatusCode gpio_init(void) {
  /* Enable GPIO clocks and disable JTAG, using only SWD. */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* We are not given any GPIO PORT C/E pins on our Controller Board */
  // __HAL_RCC_GPIOC_CLK_ENABLE();
  // __HAL_RCC_GPIOE_CLK_ENABLE();

  /* Disables the JTAG (Trace interface). Uses TRACE_MODE = 00, freeing up all pins */
  MODIFY_REG(DBGMCU->CR, DBGMCU_CR_TRACE_IOEN_Msk, 0x00000000U);
  return STATUS_CODE_OK;
}

StatusCode gpio_init_pin(const GpioAddress *address, const GpioMode pin_mode,
                         GpioState init_state) {
  if (address == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT ||
      pin_mode >= NUM_GPIO_MODES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  /* Don't allow SWD/SCLK to be configured */
  if (address->port == GPIO_PORT_A && (address->pin == 13 || address->pin == 14)) {
    return STATUS_CODE_INVALID_ARGS;
  }

  taskENTER_CRITICAL();

  uint8_t gpio_pull = GPIO_NOPULL;
  if (pin_mode == GPIO_INPUT_PULL_DOWN) {
    gpio_pull = GPIO_PULLDOWN;
  } else if (pin_mode == GPIO_INPUT_PULL_UP) {
    gpio_pull = GPIO_PULLUP;
  }

  GPIO_InitTypeDef init = { .Pin = 1U << (address->pin),
                            .Mode = s_gpio_mode_map[pin_mode],
                            .Pull = gpio_pull,
                            .Speed = GPIO_SPEED_FREQ_HIGH,
                            .Alternate = address->alternate_func };

  GPIO_TypeDef *gpio_port = (GPIO_TypeDef *)(AHB2PERIPH_BASE + (address->port * 0x400U));
  HAL_GPIO_Init(gpio_port, &init);

  if (pin_mode == GPIO_OUTPUT_OPEN_DRAIN || pin_mode == GPIO_OUTPUT_PUSH_PULL) {
    HAL_GPIO_WritePin(gpio_port, init.Pin, init_state);
  }

  taskEXIT_CRITICAL();
  return STATUS_CODE_OK;
}

StatusCode gpio_set_state(const GpioAddress *address, GpioState state) {
  if (address == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT) {
    return STATUS_CODE_INVALID_ARGS;
  }

  taskENTER_CRITICAL();
  GPIO_TypeDef *gpio_port = (GPIO_TypeDef *)(AHB2PERIPH_BASE + (address->port * 0x400U));
  HAL_GPIO_WritePin(gpio_port, 1U << (address->pin), state);
  taskEXIT_CRITICAL();
  return STATUS_CODE_OK;
}

StatusCode gpio_toggle_state(const GpioAddress *address) {
  if (address == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  taskENTER_CRITICAL();
  GPIO_TypeDef *gpio_port = (GPIO_TypeDef *)(AHB2PERIPH_BASE + (address->port * 0x400U));
  HAL_GPIO_TogglePin(gpio_port, 1U << (address->pin));
  taskEXIT_CRITICAL();
  return STATUS_CODE_OK;
}

GpioState gpio_get_state(const GpioAddress *address) {
  if (address == NULL) {
    return GPIO_STATE_LOW;
  }

  taskENTER_CRITICAL();
  GPIO_TypeDef *gpio_port = (GPIO_TypeDef *)(AHB2PERIPH_BASE + (address->port * 0x400U));
  GPIO_PinState state = HAL_GPIO_ReadPin(gpio_port, 1U << (address->pin));
  taskEXIT_CRITICAL();
  return state;
}
