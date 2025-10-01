/************************************************************************************************
 * @file    fans.c
 *
 * @brief   Fans source file
 *
 * @date    2025-10-01
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "gpio.h"
#include "interrupts.h"
#include "status.h"
#include "pwm.h"

/* Intra-component Headers */
#include "rear_controller_hw_defs.h"
#include "rear_controller.h"

static GpioAddress fans_address = REAR_CONTROLLER_MOTOR_FAN_PWM;

static RearControllerStorage *rear_controller_storage;

StatusCode fans_init(RearControllerStorage *storage) {
    if (storage == NULL) {
        return STATUS_CODE_INVALID_ARGS;
    }
    StatusCode status = gpio_init_pin(&fans_address, GPIO_ALTFN_PUSH_PULL, GPIO_STATE_LOW);
    if (status != STATUS_CODE_OK) {
        return status;
    }
    status = pwm_init(TIM3_CH1, 40000);
    if (status != STATUS_CODE_OK) {
        return status;
    }
    // ensure fan is off initially
    status = pwm_set_dc(TIM3_CH1, 0, PWM_CHANNEL_1, false);
    if (status != STATUS_CODE_OK) {
        return status;
    }
    storage->max_board_temperature = 0;
    storage->max_cell_temperature = 0;
}

StatusCode fans_run() {
    if (rear_controller_storage == NULL) {
        return STATUS_CODE_UNINITIALIZED;
    }
    if (storage->max_board_temperature == FAN_TEMP_INVALID || storage->max_cell_temperature  == FAN_TEMP_INVALID) {
        return STATUS_CODE_UNINITIALIZED;
    }
    if (rear_controller_storage->max_board_temperature >= FAN_TEMP_THRESHOLD_C || rear_controller_storage->max_cell_temperature >= FAN_TEMP_THRESHOLD_C) {
        return pwm_set_dc(TIM3_CH1, 100, PWM_CHANNEL_1, false);
    } else {
        return pwm_set_dc(TIM3_CH1, 0, PWM_CHANNEL_1, false);
    }
}
