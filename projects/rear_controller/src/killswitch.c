#include "killswitch.h"
#include "rear_fw_defs.h"
#include "interrupts.h"
#include "status.h"
#include "gpio.h"
#include "delay.h"

GpioAddress killswitch_address = REAR_CONTROLLER_KILLSWITCH_GPIO;

InterruptSettings killswitch_settings = {
    INTERRUPT_TYPE_INTERRUPT,
    INTERRUPT_PRIORITY_NORMAL,
    INTERRUPT_EDGE_RISING,
};

StatusCode killswitch_init( Event event, const Task *task){
    gpio_init_pin(&killswitch_address, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);

        GpioState state;
    delay_ms(10);
    state = gpio_get_state(&killswitch_address);

    if (state == GPIO_STATE_LOW){
        gpio_register_interrupt(&killswitch_address, &killswitch_settings, KILLSWITCH_EVENT, task);
    }

    else {} // TO DO

    return STATUS_CODE_OK;
}