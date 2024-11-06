/************************************************************************************************
 * uart.c
 *
 * UART Library Source Code
 *
 * Created: 2024-11-02
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */
#include "stm32l433xx.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_rcc.h"
#include "stm32l4xx_hal_uart.h"

/* Intra-component Headers */
#include "gpio.h"
#include "interrupts.h"
#include "queues.h"
#include "status.h"
#include "uart.h"
#include "uart_mcu.h"

static inline void prv_enable_usart1(void) {
  __HAL_RCC_USART1_CLK_ENABLE();
}
static inline void prv_enable_usart2(void) {
  __HAL_RCC_USART2_CLK_ENABLE();
}

static const uint16_t s_uart_flow_control_map[] = {
  [UART_FLOW_CONTROL_NONE] = UART_HWCONTROL_NONE,
  [UART_FLOW_CONTROL_RTS] = UART_HWCONTROL_RTS,
  [UART_FLOW_CONTROL_CTS] = UART_HWCONTROL_CTS,
  [UART_FLOW_CONTROL_RTS_CTS] = UART_HWCONTROL_RTS_CTS,
};

typedef struct UartPortQueue {
  Queue rx_queue;
  Queue tx_queue;
  uint8_t rx_buf[UART_MAX_BUFFER_LEN];
  uint8_t tx_buf[UART_MAX_BUFFER_LEN];
} UartPortQueue;

typedef struct {
  USART_TypeDef *base;
  void (*rcc_cmd)(void);
  uint8_t irq;
} UartPortData;

static UartPortData s_port[] = {
  [UART_PORT_1] = { .rcc_cmd = prv_enable_usart1, .irq = USART1_IRQn, .base = USART1 },
  [UART_PORT_2] = { .rcc_cmd = prv_enable_usart2, .irq = USART2_IRQn, .base = USART2 },
};

static UART_HandleTypeDef s_uart_handles[NUM_UART_PORTS];
static UartPortQueue s_port_queues[NUM_UART_PORTS];
static uint8_t rx_bytes[NUM_UART_PORTS];

void USART1_IRQHandler(void) {
  HAL_UART_IRQHandler(&s_uart_handles[UART_PORT_1]);
}

void USART2_IRQHandler(void) {
  HAL_UART_IRQHandler(&s_uart_handles[UART_PORT_2]);
}

/* Callback functions for HAL UART TX */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  UartPort uart = NUM_UART_PORTS;
  for (UartPort i = 0; i < NUM_UART_PORTS; i++) {
    if (&s_uart_handles[i] == huart) {
      uart = i;
    }
  }

  if (uart >= NUM_UART_PORTS) {
    return;
  }

  uint8_t tx_byte;

  if (xQueueReceiveFromISR(s_port_queues[uart].tx_queue.handle, &tx_byte, pdFALSE) == pdFALSE) {
    /* If there is no more data */
    __HAL_UART_DISABLE_IT(huart, UART_IT_TXE);
  } else {
    HAL_UART_Transmit_IT(huart, &tx_byte, 1);
  }
  __HAL_UART_CLEAR_IT(&s_uart_handles[uart], UART_IT_TXE);
}

/* Callback functions for HAL UART RX */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  UartPort uart = NUM_UART_PORTS;
  for (UartPort i = 0; i < NUM_UART_PORTS; i++) {
    if (&s_uart_handles[i] == huart) {
      uart = i;
    }
  }

  if (uart >= NUM_UART_PORTS) {
    return;
  }

  uint8_t *rx_byte = &rx_bytes[uart];

  HAL_UART_Receive_IT(huart, rx_byte, 1);
  if (xQueueSendFromISR(s_port_queues[uart].rx_queue.handle, &rx_byte, pdFALSE) == errQUEUE_FULL) {
    /* Drop oldest data if queue is full */
    uint8_t buf = 0;
    xQueueReceiveFromISR(s_port_queues[uart].rx_queue.handle, &buf, pdFALSE);
    xQueueSendFromISR(s_port_queues[uart].rx_queue.handle, rx_byte, pdFALSE);
  }

  __HAL_UART_CLEAR_IT(&s_uart_handles[uart], UART_IT_RXNE);
}

StatusCode uart_init(UartPort uart, UartSettings *settings) {
  if (settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (uart >= NUM_UART_PORTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_port_queues[uart].tx_queue.item_size = sizeof(uint8_t);
  s_port_queues[uart].tx_queue.num_items = UART_MAX_BUFFER_LEN;
  s_port_queues[uart].tx_queue.storage_buf = s_port_queues[uart].tx_buf;
  queue_init(&s_port_queues[uart].tx_queue);

  s_port_queues[uart].rx_queue.item_size = sizeof(uint8_t);
  s_port_queues[uart].rx_queue.num_items = UART_MAX_BUFFER_LEN;
  s_port_queues[uart].rx_queue.storage_buf = s_port_queues[uart].rx_buf;
  queue_init(&s_port_queues[uart].rx_queue);

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

  s_port[uart].rcc_cmd();

  if (HAL_UART_Init(&s_uart_handles[uart]) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  /* Initialize interrupts */
  interrupt_nvic_enable(s_port[uart].irq, INTERRUPT_PRIORITY_HIGH);

  HAL_UART_Receive_IT(&s_uart_handles[uart], &rx_bytes[uart], 1);

  return STATUS_CODE_OK;
}

StatusCode uart_rx(UartPort uart, uint8_t *data, size_t len) {
  if (data == NULL || uart >= NUM_UART_PORTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (len > (size_t)queue_get_spaces_available(&s_port_queues[uart].tx_queue)) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  for (uint8_t i = 0; i < len; i++) {
    if (queue_receive(&s_port_queues[uart].tx_queue, &data[i], 0) != STATUS_CODE_OK) {
      return STATUS_CODE_INCOMPLETE;
    }
  }

  return STATUS_CODE_OK;
}

StatusCode uart_tx(UartPort uart, uint8_t *data, size_t len) {
  if (data == NULL || uart >= NUM_UART_PORTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (len > (size_t)queue_get_spaces_available(&s_port_queues[uart].tx_queue)) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  StatusCode status = STATUS_CODE_OK;

  for (uint8_t i = 0; i < len; i++) {
    if (queue_send(&s_port_queues[uart].tx_queue, &data[i], 0) != STATUS_CODE_OK) {
      status = STATUS_CODE_INCOMPLETE;
      break;
    }
  }

  if (status == STATUS_CODE_OK) {
    __HAL_UART_ENABLE_IT(&s_uart_handles[uart], UART_IT_TXE);
  }

  return status;
}
