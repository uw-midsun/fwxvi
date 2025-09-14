/* Standard library Headers */
#include <math.h>

/* Inter-component Headers */
#include "adc.h"
#include "gpio.h"
#include "log.h"
#include "pedal_calib.h"

/* Intra-component Headers */
#include "brake_pedal.h"
#include "front_controller_hw_defs.h"

static GpioAddress s_brake_gpio = FRONT_CONTROLLER_BRAKE_PEDAL;
static FrontControllerStorage *front_controller_storage;
static BrakePedalStorage brake_storage = { 0U };

StatusCode brake_pedal_init(FrontControllerStorage *storage){
    if(storage == NULL){
        return STATUS_CODE_INVALID_ARGS;
    }

    gpio_init_pin(&s_brake_gpio, GPIO_ANALOG, GPIO_STATE_LOW);
    adc_add_channel(&s_brake_gpio);
    return STATUS_CODE_OK;
}

StatusCode brake_pedal_run(){
    if(front_controller_storage == NULL){
        return STATUS_CODE_UNINITIALIZED;
    }

    // Normalize adc_reading between lower and upper thresholds
    uint16_t adc_reading = brake_storage.calibration_data.lower_value
    adc_read_raw(&s_brake_gpio, &adc_reading);
    float calc_reading = ((float)adc_reading - (float)brake_storage.calibration_data.lower_value)/((float)brake_storage.calibration_data.upper_value - (float)brake_storage.calibration_data.lower_value);
    brake_storage.prev_reading = calc_reading * front_controller_storage -> brake_low_pass_filter_alpha + (1 - front_controller_storage -> brake_low_pass_filter_alpha) * brake_storage.prev_reading;

    // Update front controller storage to enable brakes if the reading is above deadzone 
    if (brake_storage.prev_reading > deadzone){
        front_controller_storage -> brake_enabled = true;
    } else {
        front_controller_storage -> brake_enabled = false;
    }

    return STATUS_CODE_OK;

}