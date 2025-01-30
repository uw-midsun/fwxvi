#include "pwm.h"

#include "status.h"
#include "stm32l4xx_hal_tim.h"
#include "stm32l4xx_hal.h"
#define NUM_CHANNELS 4

static TIM_HandleTypeDef htim[NUM_PWM_TIMERS];
static uint16_t s_period_us[NUM_PWM_TIMERS] = {0};

static void prv_enable_periph_clock(PwmTimer timer) {
    switch (timer) {
        case PWM_TIMER_1:
            __HAL_RCC_TIM1_CLK_ENABLE();
            break;
        case PWM_TIMER_2:
            __HAL_RCC_TIM2_CLK_ENABLE();
            break;
        case PWM_TIMER_6:
            __HAL_RCC_TIM6_CLK_ENABLE();
            break;
        case PWM_TIMER_7:
            __HAL_RCC_TIM7_CLK_ENABLE();
            break;
         
        case PWM_TIMER_15:
            __HAL_RCC_TIM15_CLK_ENABLE();
            break;
        case PWM_TIMER_16:
            __HAL_RCC_TIM16_CLK_ENABLE();
            break;
        default:
            break;
    }
}

StatusCode pwm_init(PwmTimer timer, uint16_t period_us) {
    if (timer >= NUM_PWM_TIMERS || period_us == 0) {
        return STATUS_CODE_INVALID_ARGS;
    }

    prv_enable_periph_clock(timer);

    htim[timer].Instance = (timer == PWM_TIMER_1) ? TIM1 : 
                           (timer == PWM_TIMER_2) ? TIM2 : 
                           (timer == PWM_TIMER_6) ? TIM6 : 
                            (timer == PWM_TIMER_7) ? TIM7 :
                           (timer == PWM_TIMER_15) ? TIM15 : 
                            (timer== PWM_TIMER_16) ? TIM16 : 
    htim[timer].Init.Prescaler = (HAL_RCC_GetPCLK1Freq() / 1000000) - 1;
    htim[timer].Init.CounterMode = TIM_COUNTERMODE_UP;
    htim[timer].Init.Period = period_us - 1;
    htim[timer].Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim[timer].Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_PWM_Init(&htim[timer]) != HAL_OK) {
        return STATUS_CODE_INTERNAL_ERROR;
    }

    s_period_us[timer] = period_us;
    return STATUS_CODE_OK;
}

StatusCode pwm_set_pulse(PwmTimer timer, uint16_t pulse_width_us, uint8_t channel, bool n_channel_en) {
    if (timer >= NUM_PWM_TIMERS || pulse_width_us > s_period_us[timer]|| s_period_us[timer]==0) {
        return STATUS_CODE_INVALID_ARGS;
    }

    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = pulse_width_us;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    if (HAL_TIM_PWM_ConfigChannel(&htim[timer], &sConfigOC, channel) != HAL_OK) {
        return STATUS_CODE_INTERNAL_ERROR;
    }

    HAL_TIM_PWM_Start(&htim[timer], channel);
    return STATUS_CODE_OK;
}

StatusCode pwm_set_dc(PwmTimer timer, uint16_t dc, uint8_t channel, bool n_channel_en) {
    if (timer >= NUM_PWM_TIMERS || dc > 100) {
        return STATUS_CODE_INVALID_ARGS;
    }

    uint16_t pulse_width = ((s_period_us[timer]+1) * dc) / 100-1;
    return pwm_set_pulse(timer, pulse_width, channel, n_channel_en);
}

uint16_t pwm_get_period(PwmTimer timer) {
    return (timer < NUM_PWM_TIMERS) ? s_period_us[timer] : 0;
}
