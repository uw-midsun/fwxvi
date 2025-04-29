/************************************************************************************************
 * @file   adc.c
 *
 * @brief  ADC Library Source code
 *
 * @date   2024-11-03
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include "stm32l4xx_hal_conf.h"

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "ms_semaphore.h"
#include "stm32l433xx.h"
#include "stm32l4xx_hal_adc.h"
#include "stm32l4xx_hal_dma.h"

/* Intra-component Headers */
#include "adc.h"
#include "interrupts.h"

#define VREFINT_MV 3300U
#define ADC_MAX_VAL 4095U
#define MAX_ADC_READINGS 16U

typedef struct AdcStatus {
  bool initialized;
  uint8_t active_channels;
  SemaphoreHandle_t converting_handle;
  StaticSemaphore_t converting_semaphore;
} AdcStatus;

/* ADC1 peripheral status */
static AdcStatus s_adc_status;

/* ADC1 handler */
static ADC_HandleTypeDef s_adc_handle;

/* DMA1 handler */
static DMA_HandleTypeDef s_dma_handle;

/* Array of readings, in order of rank, which DMA will write to */
static volatile uint16_t s_adc_readings[MAX_ADC_READINGS];

/* Store of ranks for each channel, at their channel index */
static uint8_t s_adc_ranks[NUM_ADC_CHANNELS];

/* Convert GPIO address to one of the 16 ADC channels */
/* Channels 1-4 are occupied by pins C0-C3 */
/* Channels 5-12 are occupied by pins A0-A7 */
/* Channels 13-14 are occupied by pins C4-C5 */
/* Channels 15-16 are occupied by pins B0-B1 */
static StatusCode s_adc_get_channel(GpioAddress *address, uint8_t *adc_channel) {
  *adc_channel = address->pin;
  switch (address->port) {
    case GPIO_PORT_A:
      if (address->pin > 7U) {
        return STATUS_CODE_INVALID_ARGS;
      }
      *adc_channel += 5U;
      break;
    case GPIO_PORT_B:
      if (address->pin > 1U) {
        return STATUS_CODE_INVALID_ARGS;
      }
      *adc_channel += 15U;
      break;
    case GPIO_PORT_C:
      if (address->pin > 5U) {
        return STATUS_CODE_INVALID_ARGS;
      }
      if (address->pin < 4U) {
        *adc_channel += 1U;
      } else {
        *adc_channel += 9U;
      }
      break;
    default:
      return STATUS_CODE_INVALID_ARGS;
  }

  if (*adc_channel < 1U || *adc_channel > NUM_ADC_CHANNELS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  return STATUS_CODE_OK;
}

static StatusCode s_check_channel_enabled(uint8_t channel) {
  if (s_adc_ranks[channel] != 0U) {
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_UNINITIALIZED;
  }
}

StatusCode adc_init(void) {
  if (s_adc_status.initialized) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_adc_status.active_channels == 0) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  __HAL_RCC_ADC_CLK_ENABLE();

  /* Initialize ADC1 */
  s_adc_handle.Instance = ADC1;
  s_adc_handle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  s_adc_handle.Init.Resolution = ADC_RESOLUTION12b;
  s_adc_handle.Init.ScanConvMode = ENABLE; /* Perform multiple conversions in the sequence of channel ranks */
  s_adc_handle.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  s_adc_handle.Init.LowPowerAutoWait = DISABLE;
  s_adc_handle.Init.ContinuousConvMode = DISABLE;
  s_adc_handle.Init.NbrOfConversion = s_adc_status.active_channels;
  s_adc_handle.Init.DiscontinuousConvMode = DISABLE;
  s_adc_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  s_adc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIG_EDGE_NONE;
  s_adc_handle.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  s_adc_handle.Init.OversamplingMode = DISABLE;

  if (HAL_ADC_Init(&s_adc_handle) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  for (uint8_t i = 0; i < NUM_ADC_CHANNELS; i++) {
    if (s_check_channel_enabled(i) == STATUS_CODE_OK) {
      ADC_ChannelConfTypeDef channel_conf = { 0 };
      channel_conf.Channel = i;
      channel_conf.Rank = i + 1U;
      channel_conf.SamplingTime = ADC_SAMPLETIME_24CYCLES_5;
      channel_conf.SingleDiff = ADC_SINGLE_ENDED;
      channel_conf.OffsetNumber = ADC_OFFSET_NONE;

      if (HAL_ADC_ConfigChannel(&s_adc_handle, &channel_conf) != HAL_OK) {
        return STATUS_CODE_INTERNAL_ERROR;
      }
    }
  }

  /* Initialze DMA1 */
  __HAL_RCC_DMA1_CLK_ENABLE();
  s_dma_handle.Instance = DMA1_Channel1;
  s_dma_handle.Init.Direction = DMA_PERIPH_TO_MEMORY;
  s_dma_handle.Init.PeriphInc = DMA_PINC_DISABLE;
  s_dma_handle.Init.MemInc = DMA_MINC_ENABLE;
  s_dma_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  s_dma_handle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
  s_dma_handle.Init.Mode = DMA_NORMAL;
  s_dma_handle.Init.Priority = DMA_PRIORITY_HIGH;

  if (HAL_DMA_Init(&s_dma_handle) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  __HAL_LINKDMA(&s_adc_handle, DMA_Handle, s_dma_handle);

  if (HAL_ADCEx_Calibration_Start(&s_adc_handle, ADC_SINGLE_ENDED) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  s_adc_status.converting_handle = xSemaphoreCreateBinaryStatic(&s_adc_status.converting_semaphore);

  if (s_adc_status.converting_handle == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  status_ok_or_return(interrupt_nvic_enable(DMA1_Channel1_IRQn, INTERRUPT_PRIORITY_HIGH));

  s_adc_status.initialized = true;

  return STATUS_CODE_OK;
}

StatusCode adc_add_channel(GpioAddress *address) {
  if (address == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_adc_status.initialized) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  uint8_t channel;
  status_ok_or_return(s_adc_get_channel(address, &channel));

  if (s_check_channel_enabled(channel) == STATUS_CODE_OK) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  if (++s_adc_status.active_channels < NUM_ADC_CHANNELS) {
    s_adc_ranks[channel] = s_adc_status.active_channels;
  } else {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  return STATUS_CODE_OK;
}

StatusCode adc_run(void) {
  if (!s_adc_status.initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (HAL_ADC_Start_DMA(&s_adc_handle, (uint32_t *)s_adc_readings, s_adc_status.active_channels) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  /* Wait for conversion to complete */
  if (xSemaphoreTake(s_adc_status.converting_handle, pdMS_TO_TICKS(ADC_TIMEOUT_MS)) != pdTRUE) {
    return STATUS_CODE_TIMEOUT;
  }

  return STATUS_CODE_OK;
}

StatusCode adc_read_raw(GpioAddress *address, uint16_t *reading) {
  if (reading == NULL || address == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint8_t channel;
  status_ok_or_return(s_adc_get_channel(address, &channel));
  status_ok_or_return(s_check_channel_enabled(channel));

  uint8_t rank = s_adc_ranks[channel];

  *reading = s_adc_readings[rank - 1U];

  return STATUS_CODE_OK;
}

StatusCode adc_read_converted(GpioAddress *address, uint16_t *reading) {
  if (reading == NULL || address == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint16_t raw_value;
  adc_read_raw(address, &raw_value);

  *reading = (raw_value * VREFINT_MV) / ADC_MAX_VAL;

  return STATUS_CODE_OK;
}

void adc_deinit(void) {}

void DMA1_Channel1_IRQHandler(void) {
  HAL_DMA_IRQHandler(&s_dma_handle);
}

/* ADC Conversion complete callback */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  BaseType_t higher_prio;
  xSemaphoreGiveFromISR(s_adc_status.converting_handle, &higher_prio);
}
