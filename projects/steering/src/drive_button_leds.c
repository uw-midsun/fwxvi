#include "drive_button_leds.h"

LED_DATA_TYPE  LED_DATA[NUM_LEDS];
uint8_t        DMA_BUF[DMA_BUF_LEN];
volatile uint8_t DMA_READY_FLAG = 0;

void drive_button_led_init() {

  for ( uint8_t bufIndex = 0 ; bufIndex < DMA_BUF_LEN ; bufIndex++ ) {
    DMA_BUF[bufIndex] = 0;
  }

  DMA_READY_FLAG = 1;

  //return HAL_TIM_PWM_Init(&TIM);
}

void drive_button_led_set_color( uint8_t index, uint8_t r, uint8_t g, uint8_t b ) {
  LED_DATA[index].color.r = r;
  LED_DATA[index].color.g = g;
  LED_DATA[index].color.b = b;
}

void drive_button_led_update() {

  if ( !DMA_READY_FLAG ) {
    return;     //Could return a Status here
  }

  uint16_t bufIndex = 0;

  for ( uint8_t ledIndex = 0; ledIndex < NUM_LEDS; ledIndex++ ) {

    for ( uint8_t bitIndex = 0 ; bitIndex < BITS_PER_LED; bitIndex++ ) {

      uint32_t mask = ( 1 << ( BITS_PER_LED - 1 - bitIndex ) );

      DMA_BUF[ bufIndex ] = ( LED_DATA[ledIndex].data & mask ) ? HI_VAL : LOW_VAL;

      bufIndex++;

    }

  }

  if ( HAL_TIM_PWM_Start_DMA( &TIM, TIM_CHANNEL, (uint32_t *) DMA_BUF, DMA_BUF_LEN ) == HAL_OK ) {
    DMA_READY_FLAG = 0;
  }
}

void drive_button_callback() {

  HAL_TIM_PWM_Stop_DMA( &TIM, TIM_CHANNEL );
  DMA_READY_FLAG = 1;

}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
  drive_button_callback();
}