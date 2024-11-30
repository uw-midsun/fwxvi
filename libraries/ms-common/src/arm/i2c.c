/************************************************************************************************
 * @file   i2c.c
 *
 * @brief  I2C Library Source Code
 *
 * @date   2024-11-29
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "semphr.h"
#include "stm32l433xx.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_i2c.h"
#include "stm32l4xx_hal_i2c_ex.h"
#include "stm32l4xx_hal_rcc.h"

/* Intra-component Headers */
#include "i2c.h"
#include "interrupts.h"

static inline void s_enable_i2c1(void) {
  __HAL_RCC_I2C1_CLK_ENABLE();
}
static inline void s_enable_i2c2(void) {
  __HAL_RCC_I2C2_CLK_ENABLE();
}

typedef struct {
  I2C_TypeDef *base;
  void (*rcc_cmd)(void);
  uint8_t ev_irqn;
  uint8_t err_irqn;
  bool initialized;
} I2CPortData;

static I2CPortData s_port[NUM_I2C_PORTS] = {
  [I2C_PORT_1] = { .rcc_cmd = s_enable_i2c1,
                   .base = I2C1,
                   .ev_irqn = I2C1_EV_IRQn,
                   .err_irqn = I2C1_ER_IRQn },
  [I2C_PORT_2] = { .rcc_cmd = s_enable_i2c2,
                   .base = I2C2,
                   .ev_irqn = I2C2_EV_IRQn,
                   .err_irqn = I2C2_ER_IRQn },
};

static uint32_t s_i2c_timing[NUM_I2C_SPEEDS] = {
  [I2C_SPEED_STANDARD] = 0x10909CECU,
  [I2C_SPEED_FAST]     = 0x00702991U,
};

static I2C_HandleTypeDef s_i2c_handles[NUM_I2C_PORTS];

/* Mutex for port access */
static StaticSemaphore_t s_i2c_port_mutex[NUM_I2C_PORTS];
static SemaphoreHandle_t s_i2c_port_handle[NUM_I2C_PORTS];

/* Semaphore to signal event complete */
static StaticSemaphore_t s_i2c_cmplt_sem[NUM_I2C_PORTS];
static SemaphoreHandle_t s_i2c_cmplt_handle[NUM_I2C_PORTS];


/* Private helper for common TX/RX operations */
// static StatusCode s_i2c_transfer(I2CPort i2c, uint8_t *data, size_t len, bool is_rx) {
//   if (data == NULL || i2c >= NUM_I2C_PORTS || len > I2C_MAX_NUM_DATA) {
//     return STATUS_CODE_INVALID_ARGS;
//   }

//   if (!s_port[i2c].initialized) {
//     return STATUS_CODE_UNINITIALIZED;
//   }

//   /* Take the mutex for this uart port */
//   if (xSemaphoreTake(s_i2c_port_handle[i2c], pdMS_TO_TICKS(UART_TIMEOUT_MS)) != pdTRUE) {
//     return STATUS_CODE_TIMEOUT;
//   }

//   HAL_StatusTypeDef status;
//   if (is_rx) {
//     status = HAL_UART_Receive_IT(&s_i2c_handles[i2c], data, len);
//   } else {
//     status = HAL_UART_Transmit_IT(&s_i2c_handles[i2c], data, len);
//   }

//   if (status != HAL_OK) {
//     xSemaphoreGive(s_uart_port_handle[uart]);
//     return STATUS_CODE_INTERNAL_ERROR;
//   }

//   if (xSemaphoreTake(s_i2c_cmplt_handle[uart], pdMS_TO_TICKS(UART_TIMEOUT_MS)) != pdTRUE) {
//     xSemaphoreGive(s_uart_port_handle[uart]);
//     return STATUS_CODE_TIMEOUT;
//   }

//   xSemaphoreGive(s_uart_port_handle[uart]);
//   return STATUS_CODE_OK;
// }

/* Private helper to handle transfer complete */
static void s_i2c_transfer_complete_callback(I2C_HandleTypeDef *hi2c, bool is_rx) {
  BaseType_t higher_priority_task = pdFALSE;
  I2CPort i2c = NUM_I2C_PORTS;

  for (I2CPort i = 0; i < NUM_I2C_PORTS; i++) {
    if (&s_i2c_handles[i] == hi2c) {
      i2c = i;
      break;
    }
  }

  if (i2c >= NUM_I2C_PORTS) {
    return;
  }

  // __HAL_UART_CLEAR_IT(hi2c, is_rx ? I2C_IT_RXI : I2C_IT_TXI);

  xSemaphoreGiveFromISR(s_i2c_cmplt_handle[i2c], &higher_priority_task);
  portYIELD_FROM_ISR(higher_priority_task);
}

// void I2C1_EV_IRQHandler(void) {
//   HAL_I2C_IRQHandler(&s_i2c_handles[I2C_PORT_1]);
// }

// void I2C1_ER_IRQHandler(void) {
//   HAL_I2C_IRQHandler(&s_i2c_handles[I2C_PORT_1]);
// }

// void I2C2_EV_IRQHandler(void) {
//   HAL_I2C_IRQHandler(&s_i2c_handles[I2C_PORT_2]);
// }

// void I2C2_ER_IRQHandler(void) {
//   HAL_I2C_IRQHandler(&s_i2c_handles[I2C_PORT_2]);
// }

/* Callback functions for HAL I2C TX */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {
  s_i2c_transfer_complete_callback(hi2c, false);
}

/* Callback functions for HAL I2C RX */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) {
  s_i2c_transfer_complete_callback(hi2c, true);
}

StatusCode i2c_init(I2CPort i2c, const I2CSettings *settings) {
  if (settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (i2c >= NUM_I2C_PORTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_port[i2c].initialized) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  s_i2c_port_handle[i2c] = xSemaphoreCreateMutexStatic(&s_i2c_port_mutex[i2c]);
  s_i2c_cmplt_handle[i2c] = xSemaphoreCreateBinaryStatic(&s_i2c_cmplt_sem[i2c]);

  if (s_i2c_port_handle[i2c] == NULL || s_i2c_cmplt_handle[i2c] == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  gpio_init_pin_af(&settings->sda, GPIO_ALTFN_OPEN_DRAIN, GPIO_ALT4_I2C1);
  gpio_init_pin_af(&settings->scl, GPIO_ALTFN_OPEN_DRAIN, GPIO_ALT4_I2C1);

  s_i2c_handles[i2c].Instance = s_port[i2c].base;
  s_i2c_handles[i2c].Init.Timing = s_i2c_timing[settings->speed];
  s_i2c_handles[i2c].Init.OwnAddress1 = 0U;
  s_i2c_handles[i2c].Init.OwnAddress2 = 0U;
  s_i2c_handles[i2c].Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  s_i2c_handles[i2c].Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  s_i2c_handles[i2c].Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  s_i2c_handles[i2c].Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  s_i2c_handles[i2c].Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  // RCC_PeriphCLKInitTypeDef periph_clk_init = RCC_PERIPHCLK_I2C1;

  s_port[i2c].rcc_cmd();

  if (HAL_I2C_Init(&s_i2c_handles[i2c]) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  if (HAL_I2CEx_ConfigAnalogFilter(&s_i2c_handles[i2c], I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  if (HAL_I2CEx_ConfigDigitalFilter(&s_i2c_handles[i2c], 0U) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  interrupt_nvic_enable(s_port[i2c].ev_irqn, INTERRUPT_PRIORITY_NORMAL);
  interrupt_nvic_enable(s_port[i2c].err_irqn, INTERRUPT_PRIORITY_NORMAL);

  s_port[i2c].initialized = true;

  return STATUS_CODE_OK;
}

StatusCode i2c_read(I2CPort i2c, I2CAddress addr, uint8_t *rx_data, size_t rx_len) {
    return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode i2c_write(I2CPort i2c, I2CAddress addr, uint8_t *tx_data, size_t tx_len) {
    return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode i2c_read_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *rx_data, size_t rx_len) {
    return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode i2c_write_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *tx_data, size_t tx_len) {
    return STATUS_CODE_UNIMPLEMENTED;
}