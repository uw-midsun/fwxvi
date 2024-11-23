#include "pwm.h"
#include "stm32l433xx.h"


//This is to record what APB is being used for whatever timer you're initializing
typedef enum APBClk {
  APB1 = 0,
  APB2,
  NUM_APB_CLK,
} APBClk;
static TIM_HandleTypeDef *s_timer_def[NUM_PWM_TIMERS] = {
  [PWM_TIMER_1] = TIM1,    //
  [PWM_TIMER_2] = TIM2,    //
  [PWM_TIMER_6] = TIM6,    // Typically used for I2C
  [PWM_TIMER_7] = TIM7,  //
  [PWM_TIMER_15] = TIM15,  //
  [PWM_TIMER_16] = TIM16,  //
};

static uint16_t s_period_us[NUM_PWM_TIMERS] = {
  [PWM_TIMER_1] = 0,   //
  [PWM_TIMER_2] = 0,   //
  [PWM_TIMER_6] = 0,   // This timer is typically used by I2C
  [PWM_TIMER_7] = 0,  //
  [PWM_TIMER_15] = 0,  //
  [PWM_TIMER_16] = 0,  //
};

static TIM_HandleTypeDef s_timer_handles[NUM_PWM_TIMERS];

//This is based on the old fwxv clock enabling function. The HAL RCC file should also have enabling functions
static APBClk prv_enable_periph_clock(PwmTimer timer) {
  switch (timer) {
    case PWM_TIMER_1 :
      __HAL_RCC_TIM1_CLK_ENABLE();
      return APB2;
    case PWM_TIMER_2:
      __HAL_RCC_TIM2_CLK_ENABLE();
      return APB1;
    case PWM_TIMER_6:
      __HAL_RCC_TIM6_CLK_ENABLE();
      return APB1;
    case PWM_TIMER_7:
      __HAL_RCC_TIM7_CLK_ENABLE();
      return APB1;
    case PWM_TIMER_15:
      __HAL_RCC_TIM15_CLK_ENABLE();
      return APB2;
    case PWM_TIMER_16:
      __HAL_RCC_TIM16_CLK_ENABLE();
      return APB2;
    default:
      return NUM_APB_CLK;
  }
}



StatusCode pwm_init(PwmTimer timer, uint16_t period_us) {
//set duty cycle and pulse as zero. you are initializing the timer
// and setting the period (1/frequency) of the pulse. In pwm_set_pulse,
// you are setting the pulse width (how long is the signal HIGH), which
// logically can't be bigger than the period
if (timer >= NUM_PWM_TIMERS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid timer id");
  } else if (period_us == 0) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Period must be greater than 0");
  }

  s_timer_handles[timer].Init

  

  HAL_StatusTypeDef HAL_TIM_PWM_Init(&s_timer_handles[timer]);

  APBClk clk = prv_enable_periph_clock(timer);

  s_period_us[timer] = period_us;

  //then need to do some OC and and actual timer period stuff



}

uint16_t pwm_get_period(PwmTimer timer) {
//should be pretty simple. Should be a HAL function for that.
}


StatusCode pwm_set_pulse(PwmTimer timer, uint16_t pulse_width_us, uint8_t channel) {
//Set the pulse width for the given channel. Make sure that channel is being used for
// pwm first.
}


StatusCode pwm_set_dc(PwmTimer timer, uint16_t dc, uint8_t channel) {
//Sets the pulse width based on a given dc for a given channel. 
//Will screw with whatever's set by pwm_set_pulse. Check that channel is being 
// used for pwm first.
}


uint16_t pwm_get_dc(PwmTimer timer) {
//Gets the dc, likely based off of the period and pulse width.
}
