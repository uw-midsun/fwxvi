/************************************************************************************************
 * @file    bl_port_uart_xbee.c
 *
 * @brief   Real bl_port_uart_* for the telemetry board XBee, the upstream transport on a FOTA
 *          gateway. USART2 on PA2 (TX) / PA3 (RX) at 230400 8N1, no flow control, with the XBee
 *          held out of reset and awake. RX is interrupt driven into a ring buffer so the gateway
 *          superloop never drops bytes at 230400, TX is blocking
 *
 * @note    The CAN, flash, jump, region and time bl_port_* hooks are NOT here, they are shared
 *          with can_bootloader (see the shared-port decision in the FOTA README). This file only
 *          replaces the UART stubs that the leaf bootloader ships
 *
 * @date    2026-06-29
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"

/* Intra-component Headers */
#include "bl_status.h"
#include "bootloader_port.h"

/* XBee wiring on the telemetry board (mirrors telemetry_hw_defs.h) */
#define XBEE_USART USART2
#define XBEE_USART_IRQ USART2_IRQn
#define XBEE_XRST_PIN GPIO_PIN_5      /* PA5, low holds the XBee in reset */
#define XBEE_SLEEP_RQ_PIN GPIO_PIN_11 /* PA11, high requests sleep */

/* Power of two so the masks wrap cheaply, sized for one full 2 KB datagram of 0x7E framing */
#define RX_RING_SIZE 4096U
#define RX_RING_MASK (RX_RING_SIZE - 1U)

static UART_HandleTypeDef s_uart;

static volatile uint8_t s_rx_ring[RX_RING_SIZE];
static volatile uint32_t s_rx_head; /* written by the ISR */
static volatile uint32_t s_rx_tail; /* read by the superloop */

BlStatus bl_port_uart_init(uint32_t baud) {
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* PA2/PA3 to USART2 alternate function */
  GPIO_InitTypeDef af = {
    .Pin = GPIO_PIN_2 | GPIO_PIN_3,
    .Mode = GPIO_MODE_AF_PP,
    .Pull = GPIO_PULLUP,
    .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
    .Alternate = GPIO_AF7_USART2,
  };
  HAL_GPIO_Init(GPIOA, &af);

  /* Release the XBee from reset and keep it awake before any traffic */
  HAL_GPIO_WritePin(GPIOA, XBEE_XRST_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA, XBEE_SLEEP_RQ_PIN, GPIO_PIN_RESET);
  GPIO_InitTypeDef ctrl = {
    .Pin = XBEE_XRST_PIN | XBEE_SLEEP_RQ_PIN,
    .Mode = GPIO_MODE_OUTPUT_PP,
    .Pull = GPIO_NOPULL,
    .Speed = GPIO_SPEED_FREQ_LOW,
  };
  HAL_GPIO_Init(GPIOA, &ctrl);

  __HAL_RCC_USART2_CLK_ENABLE();
  s_uart.Instance = XBEE_USART;
  s_uart.Init.BaudRate = baud;
  s_uart.Init.WordLength = UART_WORDLENGTH_8B;
  s_uart.Init.StopBits = UART_STOPBITS_1;
  s_uart.Init.Parity = UART_PARITY_NONE;
  s_uart.Init.Mode = UART_MODE_TX_RX;
  s_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  s_uart.Init.OverSampling = UART_OVERSAMPLING_16;
  s_uart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  if (HAL_UART_Init(&s_uart) != HAL_OK) {
    return BL_ERR_UART;
  }

  s_rx_head = 0U;
  s_rx_tail = 0U;

  /* Drive RX off the byte interrupt directly into the ring, HAL_UART_Receive_IT is not used so
     the stream is continuous and never has to be re-armed between fragments */
  __HAL_UART_ENABLE_IT(&s_uart, UART_IT_RXNE);
  HAL_NVIC_SetPriority(XBEE_USART_IRQ, 5, 0);
  HAL_NVIC_EnableIRQ(XBEE_USART_IRQ);
  return BL_OK;
}

BlStatus bl_port_uart_rx(uint8_t *buf, uint32_t *len) {
  if ((buf == NULL) || (len == NULL)) {
    return BL_ERR_INVALID_ARGS;
  }
  uint32_t cap = *len;
  uint32_t n = 0U;
  while (n < cap && s_rx_tail != s_rx_head) {
    buf[n] = s_rx_ring[s_rx_tail & RX_RING_MASK];
    s_rx_tail++;
    n++;
  }
  *len = n;
  return BL_OK;
}

BlStatus bl_port_uart_tx(const uint8_t *buf, uint32_t len) {
  if ((buf == NULL) && (len != 0U)) {
    return BL_ERR_INVALID_ARGS;
  }
  if (HAL_UART_Transmit(&s_uart, (uint8_t *)buf, (uint16_t)len, HAL_MAX_DELAY) != HAL_OK) {
    return BL_ERR_UART;
  }
  return BL_OK;
}

void USART2_IRQHandler(void) {
  /* A read of RDR clears RXNE, clear ORE explicitly so an overrun never wedges the receiver */
  if (__HAL_UART_GET_FLAG(&s_uart, UART_FLAG_ORE)) {
    __HAL_UART_CLEAR_OREFLAG(&s_uart);
  }
  if (__HAL_UART_GET_FLAG(&s_uart, UART_FLAG_RXNE)) {
    uint8_t byte = (uint8_t)(s_uart.Instance->RDR & 0xFFU);
    uint32_t next = (s_rx_head + 1U) & RX_RING_MASK;
    if (next != (s_rx_tail & RX_RING_MASK)) {
      s_rx_ring[s_rx_head & RX_RING_MASK] = byte;
      s_rx_head++;
    }
    /* On a full ring the byte is dropped, the whole-datagram CRC then forces a retransmit */
  }
}
