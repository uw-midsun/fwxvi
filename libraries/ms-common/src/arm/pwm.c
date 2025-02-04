#include "pwm.h"
#include  "log.h"
#include "gpio.h"
#include "status.h"
#include "stm32l433xx.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_dma.h"
#include "stm32l4xx_hal_tim.h"
#include "stm32l4xx_hal.h"

#define NUM_CHANNELS 4

static TIM_HandleTypeDef s_timer_handle[NUM_PWM_TIMERS];
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
        default:
            break;
    }
}

StatusCode pwm_init(PwmTimer timer, uint16_t period_us) {
    if (timer >= NUM_PWM_TIMERS || period_us == 0) {
        LOG_DEBUG("invalid timer");
        return STATUS_CODE_INVALID_ARGS;
    }
    
    prv_enable_periph_clock(timer);
    
    switch (timer) {
        case PWM_TIMER_1:
            s_timer_handle[timer].Instance = TIM1;
            break;
        case PWM_TIMER_2:
            s_timer_handle[timer].Instance = TIM2;
            break;
        case PWM_TIMER_6:
            s_timer_handle[timer].Instance = TIM6;
            break;
        default:
            return STATUS_CODE_INVALID_ARGS;
    }
    uint32_t clock_freq = (timer == PWM_TIMER_1) ? HAL_RCC_GetPCLK2Freq() : HAL_RCC_GetPCLK1Freq();
    s_timer_handle[timer].Init.Prescaler = (clock_freq / 1000000) - 1;
    s_timer_handle[timer].Init.CounterMode = TIM_COUNTERMODE_UP;
    s_timer_handle[timer].Init.Period = period_us - 1;
    s_timer_handle[timer].Init.ClockDivision = 0U;
    s_timer_handle[timer].Init.RepetitionCounter = 0U;
    
    if (HAL_TIM_PWM_Init(&s_timer_handle[timer]) != HAL_OK) {
        return STATUS_CODE_INTERNAL_ERROR;
    }

    s_period_us[timer] = period_us;
    return STATUS_CODE_OK;
}

StatusCode pwm_set_pulse(PwmTimer timer, uint16_t pulse_width_us, PwmChannel channel, bool n_channel_en) {
    if (timer >= NUM_PWM_TIMERS || pulse_width_us > s_period_us[timer] || s_period_us[timer] == 0) {
        return STATUS_CODE_INVALID_ARGS;
    }

    TIM_OC_InitTypeDef s_config_oc = {0};
    s_config_oc.Pulse = pulse_width_us;
    s_config_oc.OCMode = TIM_OCMODE_PWM1;
    s_config_oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    s_config_oc.OCNPolarity = n_channel_en ? TIM_OCNPOLARITY_HIGH : TIM_OCNPOLARITY_LOW;
    s_config_oc.OCFastMode = TIM_OCFAST_DISABLE;
    s_config_oc.OCIdleState = TIM_OCIDLESTATE_RESET;

    if (HAL_TIM_PWM_ConfigChannel(&s_timer_handle[timer], &s_config_oc, channel * 4U) != HAL_OK) {
        return STATUS_CODE_INTERNAL_ERROR;
    }

    HAL_TIM_PWM_Start(&s_timer_handle[timer], channel * 4U);
    return STATUS_CODE_OK;
}

StatusCode pwm_set_dc(PwmTimer timer, uint16_t dc, PwmChannel channel, bool n_channel_en) {
    if (timer >= NUM_PWM_TIMERS || dc > 100) {
        return STATUS_CODE_INVALID_ARGS;
    }

    uint16_t pulse_width = ((s_period_us[timer] + 1) * dc) / 100 - 1;
    return pwm_set_pulse(timer, pulse_width, channel, n_channel_en);
}

uint16_t pwm_get_dc(PwmTimer timer, PwmChannel channel) {
    return 0;
}

uint16_t pwm_get_period(PwmTimer timer) {
    return (timer < NUM_PWM_TIMERS) ? s_period_us[timer] : 0;
}