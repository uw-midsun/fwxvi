
#include "precharge.h"
#include "rear_fw_defs.h"
#include "interrupts.h"
#include "status.h"
#include "gpio.h"
#include "delay.h"



GpioAddress precharge_address = REAR_CONTROLLER_PRECHARGE_GPIO;

InterruptSettings precharge_settings = {
    INTERRUPT_TYPE_INTERRUPT,
    INTERRUPT_PRIORITY_NORMAL,
    INTERRUPT_EDGE_FALLING,
};

StatusCode precharge_init( Event event, const Task *task){
    gpio_init_pin(&precharge_address, GPIO_INPUT_PULL_UP, GPIO_STATE_LOW);

    GpioState state;
    delay_ms(10);
    state = gpio_get_state(&precharge_address);

    if (state == GPIO_STATE_HIGH){
        gpio_register_interrupt(&precharge_address, &precharge_settings, PRECHARGE_EVENT, task);
    }
    else {} // TO DO

    return STATUS_CODE_OK;
}

