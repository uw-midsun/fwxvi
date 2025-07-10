/************************************************************************************************
 * @file   network.c
 *
 * @brief  Tx Rx UART implementation for firmware over the air (FOTA) updates
 *
 * @date   2025-03-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "stm32l433xx.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_gpio_ex.h"
#include "stm32l4xx_hal_rcc.h"
#include "stm32l4xx_hal_rcc_ex.h"
#include "stm32l4xx_hal_uart.h"

/* Intra-component Headers */
#include "network.h"
#include "network_buffer.h"

// Global variables:
NetworkBuffer *s_network_buffer; /**< Local pointer to the network buffer */
uint8_t rx_data;                 /**< Local data reference for receiving */

static uint32_t s_tx_start_time;
static bool s_is_tx;

static uint32_t s_rx_start_time;
static bool s_is_rx;

static inline void s_enable_usart1(void) {
  __HAL_RCC_USART1_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
}
static inline void s_enable_usart2(void) {
  __HAL_RCC_USART2_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
}

/** @brief  UART Port data */
typedef struct {
  USART_TypeDef *base;   /**< UART HW Base address */
  void (*rcc_cmd)(void); /**< Function pointer to enable UART clock using RCC */
  uint8_t irq;           /**< UART interrupt number */
  bool initialized;      /**< Initialized flag */
} UartPortData;

static UartPortData s_port[NUM_UART_PORTS] = {
  [UART_PORT_1] = { .rcc_cmd = s_enable_usart1, .irq = USART1_IRQn, .base = USART1, .initialized = false },
  [UART_PORT_2] = { .rcc_cmd = s_enable_usart2, .irq = USART2_IRQn, .base = USART2, .initialized = false },
};

static const uint16_t s_uart_flow_control_map[] = {
  [UART_FLOW_CONTROL_NONE] = UART_HWCONTROL_NONE,
  [UART_FLOW_CONTROL_RTS] = UART_HWCONTROL_RTS,
  [UART_FLOW_CONTROL_CTS] = UART_HWCONTROL_CTS,
  [UART_FLOW_CONTROL_RTS_CTS] = UART_HWCONTROL_RTS_CTS,
};

static UART_HandleTypeDef s_uart_handles[NUM_UART_PORTS];
static GPIO_InitTypeDef gpio_uart_handles[NUM_UART_PORTS][2];

// IRQ handler functions
void USART1_IRQHandler(void) {
  HAL_UART_IRQHandler(&s_uart_handles[UART_PORT_1]);
}

void USART2_IRQHandler(void) {
  HAL_UART_IRQHandler(&s_uart_handles[UART_PORT_2]);
}

/* Callback functions for HAL UART TX */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  s_is_tx = true;
}

/* Callback functions for HAL UART RX */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART2) {
    network_buffer_write(s_network_buffer, &rx_data);

    /* Re-arm interrupt */
    HAL_UART_Receive_IT(huart, &rx_data, sizeof(rx_data));
  }
}

bool is_network_timeout(bool is_tx) {
  if (is_tx) {
    if (!s_is_tx && HAL_GetTick() - s_tx_start_time > FOTA_UART_TIMEOUT_MS) {
      return true;
    }
  } else {
    if (!s_is_rx && HAL_GetTick() - s_rx_start_time > FOTA_UART_TIMEOUT_MS) {
      return true;
    }
  }

  return false;
}

FotaError network_read(UartPort uart, uint8_t *data, size_t len) {
  if (data == NULL || uart >= NUM_UART_PORTS || len > NETWORK_BUFFER_SIZE) {
    return FOTA_ERROR_INVALID_ARGS;
  }
  if (!s_port[uart].initialized) {
    return FOTA_ERROR_RESOURCE_EXHAUSTED;
  }

  s_is_rx = false;
  s_rx_start_time = HAL_GetTick();

  for (size_t i = 0; i < len; i++) {
    if (is_network_timeout(false)) {
      return FOTA_ERROR_INTERNAL_ERROR;
    }

    if (network_buffer_read(s_network_buffer, data + i) != FOTA_ERROR_SUCCESS) {
      return FOTA_ERROR_INTERNAL_ERROR;
    }
  }

  s_is_rx = true;
  return FOTA_ERROR_SUCCESS;
}

FotaError network_tx(UartPort uart, uint8_t *data, size_t len) {
  // check params
  if (data == NULL || uart >= NUM_UART_PORTS || len > FOTA_UART_MAX_BUFFER_LEN) {
    return FOTA_ERROR_INVALID_ARGS;
  }
  if (!s_port[uart].initialized) {
    return FOTA_ERROR_RESOURCE_EXHAUSTED;
  }
  HAL_StatusTypeDef status = HAL_UART_Transmit_IT(&s_uart_handles[uart], data, len);

  if (status != HAL_OK) {
    return FOTA_ERROR_INTERNAL_ERROR;
  }

  s_is_tx = false;
  s_tx_start_time = HAL_GetTick();

  return FOTA_ERROR_SUCCESS;
}

FotaError network_init(UartPort uart, UartSettings *settings, NetworkBuffer *network_buffer) {
  if (settings == NULL || network_buffer == NULL || uart >= NUM_UART_PORTS) {
    return FOTA_ERROR_INVALID_ARGS;
  }
  if (s_port[uart].initialized) {
    return FOTA_ERROR_RESOURCE_EXHAUSTED;
  }

  s_network_buffer = network_buffer;
  if (network_buffer_init(s_network_buffer) != FOTA_ERROR_SUCCESS) {
    return FOTA_ERROR_INTERNAL_ERROR;
  }

  s_port[uart].rcc_cmd();

  RCC_PeriphCLKInitTypeDef periph_clk_init = { 0U };

  if (uart == UART_PORT_1) {
    periph_clk_init.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    periph_clk_init.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  } else {
    periph_clk_init.PeriphClockSelection = RCC_PERIPHCLK_USART2;
    periph_clk_init.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  }

  if (HAL_RCCEx_PeriphCLKConfig(&periph_clk_init) != HAL_OK) {
    return FOTA_ERROR_INTERNAL_ERROR;
  }

  gpio_uart_handles[uart][0].Pin = settings->tx.pin;
  gpio_uart_handles[uart][0].Mode = GPIO_MODE_AF_PP;
  gpio_uart_handles[uart][0].Pull = GPIO_NOPULL;
  gpio_uart_handles[uart][0].Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_uart_handles[uart][0].Alternate = (uint8_t)0x07;
  HAL_GPIO_Init(((settings->tx.port == GPIO_PORT_A) ? GPIOA : GPIOB), &gpio_uart_handles[uart][0]);

  gpio_uart_handles[uart][1].Pin = settings->rx.pin;
  gpio_uart_handles[uart][1].Mode = GPIO_MODE_AF_PP;
  gpio_uart_handles[uart][1].Pull = GPIO_NOPULL;
  gpio_uart_handles[uart][1].Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_uart_handles[uart][1].Alternate = (uint8_t)0x07;
  HAL_GPIO_Init(((settings->rx.port == GPIO_PORT_A) ? GPIOA : GPIOB), &gpio_uart_handles[uart][1]);

  uint32_t uart_flow_control = UART_HWCONTROL_NONE;
  if (settings->flow_control) {
    uart_flow_control = s_uart_flow_control_map[settings->flow_control];
  }

  s_uart_handles[uart].Instance = s_port[uart].base;
  s_uart_handles[uart].Init.BaudRate = settings->baudrate;
  s_uart_handles[uart].Init.WordLength = UART_WORDLENGTH_8B;
  s_uart_handles[uart].Init.StopBits = UART_STOPBITS_1;
  s_uart_handles[uart].Init.Parity = UART_PARITY_NONE;
  s_uart_handles[uart].Init.Mode = UART_MODE_TX_RX;
  s_uart_handles[uart].Init.HwFlowCtl = uart_flow_control;
  s_uart_handles[uart].Init.OverSampling = UART_OVERSAMPLING_16;

  if (HAL_UART_Init(&s_uart_handles[uart]) != HAL_OK) {
    return FOTA_ERROR_INTERNAL_ERROR;
  }

  /* Initialize interrupts, using highest priority */
  HAL_NVIC_SetPriority(s_port[uart].irq, 0, 0);
  HAL_NVIC_EnableIRQ(s_port[uart].irq);

  /* Arm the RX interrupt */
  HAL_UART_Receive_IT(&s_uart_handles[uart], &rx_data, sizeof(rx_data));

  s_port[uart].initialized = true;

  return FOTA_ERROR_SUCCESS;
}
