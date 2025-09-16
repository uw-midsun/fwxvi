#include <stdint.h>
// #include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_tim.h"

#define NUM_LEDS        6

#define TIM             htim2    //Timer
#define TIM_CHANNEL     TIM_CHANNEL_3    //Channel

#define HI_VAL          20    //0.25 us. Depends on ARR (Counter Period) value
#define LOW_VAL         68    //0.85 us. Depends on ARR (Counter Period) value

#define RESET_PERIODS   64    //80 us

#define BITS_PER_LED    24

#define DMA_BUF_LEN     ( ( BITS_PER_LED * NUM_LEDS ) + RESET_PERIODS )

typedef union {

  struct {
    uint8_t b;
    uint8_t r;
    uint8_t g;
  } color;
  
  uint32_t data;

} LED_DATA_TYPE;

extern            LED_DATA_TYPE  LED_DATA[NUM_LEDS];
extern            uint8_t        DMA_BUF[DMA_BUF_LEN];
extern volatile   uint8_t        DMA_READY_FLAG;

void drive_button_led_init();
void drive_button_led_set_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
void drive_button_led_update();
void drive_button_callback();