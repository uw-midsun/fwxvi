/************************************************************************************************
 * @file    buzzer.c
 *
 * @brief   Source file for the steering buzzer
 *
 * @date    2025-07-28
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>

/* Inter-component Headers */
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "pwm.h"
#include "software_timer.h"

/* Intra-component Headers */
#include "buzzer.h"

#define BUZZER_TIMER     PWM_TIMER_4
#define BUZZER_CHANNEL   PWM_CHANNEL_1
#define BUZZER_FREQ_HZ   4000U
#define BUZZER_DUTY      50U

/* Period in microseconds = 1/frequency * 1e6 */
#define BUZZER_PERIOD_US (1000000U / BUZZER_FREQ_HZ)

StatusCode buzzer_init() {
    status_ok_or_return(pwm_init(BUZZER_TIMER, BUZZER_PERIOD_US));
}

StatusCode buzzer_beep(uint16_t duration_ms) {

}
