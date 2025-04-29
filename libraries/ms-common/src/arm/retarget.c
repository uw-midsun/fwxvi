/************************************************************************************************
 * @file   retarget.c
 *
 * @brief  Retarget library for Hard fault handling
 *         Reference: https://interrupt.memfault.com/blog/cortex-m-hardfault-debug
 *
 * @date   2025-01-28
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>
#include <stdio.h>

/* Inter-component Headers */
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_uart.h"
#include "stm32l4xx_it.h"

/* Intra-component Headers */
#include "retarget.h"

static UART_HandleTypeDef hard_fault_uart;

int _write(int file, char *ptr, int len) {
  HAL_UART_Transmit(&hard_fault_uart, (uint8_t *)ptr, len, HAL_MAX_DELAY);
  return len;
}

void retarget_init() {
  /* Enable UART clock */
  __HAL_RCC_USART1_CLK_ENABLE();

  /* Debug UART is Pins B6 and B7 */
  GPIO_InitTypeDef GPIO_InitStruct = { 0 };
  GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  hard_fault_uart.Instance = USART1;
  hard_fault_uart.Init.BaudRate = 115200;
  hard_fault_uart.Init.WordLength = UART_WORDLENGTH_8B;
  hard_fault_uart.Init.StopBits = UART_STOPBITS_1;
  hard_fault_uart.Init.Parity = UART_PARITY_NONE;
  hard_fault_uart.Init.Mode = UART_MODE_TX_RX;
  hard_fault_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hard_fault_uart.Init.OverSampling = UART_OVERSAMPLING_16;

  if (HAL_UART_Init(&hard_fault_uart) != HAL_OK) {
    /* Hang the program if the UART Init fails */
    while (1) {
    }
  }
}

void HardFault_Handler() {
  /**
   * Get the appropriate stack pointer, depending on our mode,
   * and use it as the parameter to the C handler. This function will never return
   */

  __asm(
      ".syntax unified                \n"
      "MOVS   R0, #4                  \n" /* Move 0b0100 to R0 */
      "MOV    R1, LR                  \n" /* Move link-regsiter to R1 */
      "TST    R0, R1                  \n" /* Test link-register value to see if 3rd bit is set (SPSEL bit) */
      "BEQ    _MSP                    \n" /* If the 3rd bit is set, the MSP is being used for the SP */
      "MRS    R0, PSP                 \n" /* Move process stack pointer into R0 */
      "B      HardFault_HandlerC      \n" /* Branch to C version */
      "_MSP:                          \n" /* MSP function label */
      "MRS    R0, MSP                 \n" /* Move main stack pointer into R0 */
      "B      HardFault_HandlerC      \n" /* Branch to C version */
      ".syntax divided                \n");
}

void HardFault_HandlerC(uint32_t *hardfault_args) {
  /* Disable all interrupts and initialize UART */
  __disable_irq();
  retarget_init();

  /* When an ARM device hardfaults, the registers are automatically pushed onto the stack in the following order */
  volatile uint32_t stacked_r0 = hardfault_args[0];
  volatile uint32_t stacked_r1 = hardfault_args[1];
  volatile uint32_t stacked_r2 = hardfault_args[2];
  volatile uint32_t stacked_r3 = hardfault_args[3];
  volatile uint32_t stacked_r12 = hardfault_args[4];
  volatile uint32_t stacked_lr = hardfault_args[5];
  volatile uint32_t stacked_pc = hardfault_args[6];
  volatile uint32_t stacked_psr = hardfault_args[7];

  volatile uint32_t _CFSR = (*((volatile uint32_t *)(0xE000ED28U)));
  volatile uint32_t _HFSR = (*((volatile uint32_t *)(0xE000ED2CU)));
  volatile uint32_t _DFSR = (*((volatile uint32_t *)(0xE000ED30U)));
  volatile uint32_t _AFSR = (*((volatile uint32_t *)(0xE000ED3CU)));
  volatile uint32_t _MMAR = (*((volatile uint32_t *)(0xE000ED34U)));
  volatile uint32_t _BFAR = (*((volatile uint32_t *)(0xE000ED38U)));

  printf("R0: 0x%lx\n\r", stacked_r0);
  printf("R1: 0x%lx\n\r", stacked_r1);
  printf("R2: 0x%lx\n\r", stacked_r2);
  printf("R3: 0x%lx\n\r", stacked_r3);
  printf("R12: 0x%lx\n\r", stacked_r12);
  printf("LR: 0x%lx\n\r", stacked_lr);
  printf("PC: 0x%lx\n\r", stacked_pc);
  printf("PSR: 0x%lx\n\r", stacked_psr);
  printf("CFSR: 0x%lx\n\r", _CFSR);
  printf("HFSR: 0x%lx\n\r", _HFSR);
  printf("DFSR: 0x%lx\n\r", _DFSR);
  printf("AFSR: 0x%lx\n\r", _AFSR);
  printf("MMAR: 0x%lx\n\r", _MMAR);
  printf("BFAR: 0x%lx\n\r", _BFAR);

  __asm("BKPT #0\n\r");
}
