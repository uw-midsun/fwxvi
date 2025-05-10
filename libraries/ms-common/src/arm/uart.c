/************************************************************************************************
 * @file   uart.c
 *
 * @brief  UART Library Source Code
 *
 * @date   2024-11-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "semphr.h"
#include "stm32l433xx.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_rcc.h"
#include "stm32l4xx_hal_rcc_ex.h"
#include "stm32l4xx_hal_uart.h"

/* Intra-component Headers */
#include "gpio.h"
#include "interrupts.h"
#include "queues.h"
#include "status.h"
#include "uart.h"

static inline void s_enable_usart1(void) {
  __HAL_RCC_USART1_CLK_ENABLE();
}
static inline void s_enable_usart2(void) {
  __HAL_RCC_USART2_CLK_ENABLE();
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

/* Mutex for port access */
static StaticSemaphore_t s_uart_port_mutex[NUM_UART_PORTS];
static SemaphoreHandle_t s_uart_port_handle[NUM_UART_PORTS];

/* Semaphore to signal event complete */
static StaticSemaphore_t s_uart_cmplt_sem[NUM_UART_PORTS];
static SemaphoreHandle_t s_uart_cmplt_handle[NUM_UART_PORTS];

/* Private helper for common TX/RX operations */
static StatusCode s_uart_transfer(UartPort uart, uint8_t *data, size_t len, bool is_rx) {
  if (data == NULL || uart >= NUM_UART_PORTS || len > UART_MAX_BUFFER_LEN) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (!s_port[uart].initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  /* Take the mutex for this uart port */
  if (xSemaphoreTake(s_uart_port_handle[uart], pdMS_TO_TICKS(UART_TIMEOUT_MS)) != pdTRUE) {
    return STATUS_CODE_TIMEOUT;
  }

  HAL_StatusTypeDef status;
  if (is_rx) {
    status = HAL_UART_Receive_IT(&s_uart_handles[uart], data, len);
  } else {
    status = HAL_UART_Transmit_IT(&s_uart_handles[uart], data, len);
  }

  if (status != HAL_OK) {
    xSemaphoreGive(s_uart_port_handle[uart]);
    return STATUS_CODE_INTERNAL_ERROR;
  }

  if (xSemaphoreTake(s_uart_cmplt_handle[uart], pdMS_TO_TICKS(UART_TIMEOUT_MS)) != pdTRUE) {
    xSemaphoreGive(s_uart_port_handle[uart]);
    return STATUS_CODE_TIMEOUT;
  }

  xSemaphoreGive(s_uart_port_handle[uart]);
  return STATUS_CODE_OK;
}

/* Private helper to handle transfer complete */
static void s_uart_transfer_complete_callback(UART_HandleTypeDef *huart, bool is_rx) {
  BaseType_t higher_priority_task = pdFALSE;

  if (huart->Instance == USART1) {
    xSemaphoreGiveFromISR(s_uart_cmplt_handle[UART_PORT_1], &higher_priority_task);
  } else {
    xSemaphoreGiveFromISR(s_uart_cmplt_handle[UART_PORT_2], &higher_priority_task);
  }
  portYIELD_FROM_ISR(higher_priority_task);
}

void USART1_IRQHandler(void) {
  HAL_UART_IRQHandler(&s_uart_handles[UART_PORT_1]);
}

void USART2_IRQHandler(void) {
  HAL_UART_IRQHandler(&s_uart_handles[UART_PORT_2]);
}

/* Callback functions for HAL UART TX */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  s_uart_transfer_complete_callback(huart, false);
}

/* Callback functions for HAL UART RX */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  s_uart_transfer_complete_callback(huart, true);
}

StatusCode uart_rx(UartPort uart, uint8_t *data, size_t len) {
  return s_uart_transfer(uart, data, len, true);
}

StatusCode uart_tx(UartPort uart, uint8_t *data, size_t len) {
  return s_uart_transfer(uart, data, len, false);
}

StatusCode uart_init(UartPort uart, UartSettings *settings) {
  if (settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (uart >= NUM_UART_PORTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_port[uart].initialized) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  s_uart_port_handle[uart] = xSemaphoreCreateMutexStatic(&s_uart_port_mutex[uart]);
  s_uart_cmplt_handle[uart] = xSemaphoreCreateBinaryStatic(&s_uart_cmplt_sem[uart]);

  if (s_uart_port_handle[uart] == NULL || s_uart_cmplt_handle[uart] == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  gpio_init_pin_af(&settings->tx, GPIO_ALTFN_PUSH_PULL, GPIO_ALT7_USART1);
  gpio_init_pin_af(&settings->rx, GPIO_ALTFN_PUSH_PULL, GPIO_ALT7_USART1);

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

  RCC_PeriphCLKInitTypeDef periph_clk_init = { 0U };

  if (uart == UART_PORT_1) {
    periph_clk_init.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    periph_clk_init.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  } else {
    periph_clk_init.PeriphClockSelection = RCC_PERIPHCLK_USART2;
    periph_clk_init.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  }

  if (HAL_RCCEx_PeriphCLKConfig(&periph_clk_init) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  s_port[uart].rcc_cmd();

  if (HAL_UART_Init(&s_uart_handles[uart]) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  /* Initialize interrupts */
  interrupt_nvic_enable(s_port[uart].irq, INTERRUPT_PRIORITY_HIGH);

  s_port[uart].initialized = true;

  return STATUS_CODE_OK;
}
