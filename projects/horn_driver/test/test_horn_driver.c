/************************************************************************************************
 * @file   test_horn_driver.c
 *
 * @brief  Test file for horn_driver
 *
 * @date   2026-10-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "master_tasks.h"
#include "pwm.h"
#include "stm32l4xx_hal_tim.h"

#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "horn_driver.h"

#define TIM1_CH1 8 //PA8 Q1
#define TIM1_CH1N 7 //PA7 Q3
#define TIM1_CH2 9 //PA9 Q2
#define TIM1_CH2N 0 //PB0 Q4

GpioAddress pa8 = {
  .pin = TIM1_CH1,
  .port = GPIO_PORT_A,
};
GpioAddress pa7 = {
  .pin = TIM1_CH1N,
  .port = GPIO_PORT_A,
};
GpioAddress pa9 = {
  .pin = TIM1_CH2,
  .port = GPIO_PORT_A,
};
GpioAddress pb0 = {
  .pin = TIM1_CH2N,
  .port = GPIO_PORT_B,
};


void setup_test(void) {}

void teardown_test(void) {}

//test micro second delays are correct
//set up a software timer and do a 1 second callback
void test_horn_driver_delay() {
    gpio_get_state()
}

//test both gates not on at same time
void test_horn_driver_pins(void) {
    //check different gates not on
    TEST_ASSERT_EQUAL(,);
}

//test 

void test_example(void) {
    TEST_ASSERT_TRUE(true);
}