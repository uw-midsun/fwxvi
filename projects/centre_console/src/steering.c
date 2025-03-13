
    #include "steering.h"

    static struct SteeringStorage steering_storage = {
        .adc_readings = 0,
        .cc_inputs = 0,
        .turn_signal_adc = ADC_TURN_SIGNAL,
        .cc_adc = ADC_CRUISE_CONTROL,
        .toggle_gpio = CRUISE_CONTROL_BUTTON
    };

    static CentreConsoleStorage *centre_console_storage;

    StatusCode steering_init(CentreConsoleStorage *storage) {
        if (storage == NULL) {
            return STATUS_CODE_INVALID_ARGS;
        }

        centre_console_storage = storage;
        centre_console_storage->steering_storage = &steering_storage;

        gpio_init_pin(&centre_console_storage->steering_storage->turn_signal_adc, GPIO_ANALOG, GPIO_STATE_LOW);
        gpio_init_pin(&centre_console_storage->steering_storage->cc_adc, GPIO_ANALOG, GPIO_STATE_LOW);

        adc_add_channel(&centre_console_storage->steering_storage->turn_signal_adc);
        adc_add_channel(&centre_console_storage->steering_storage->cc_adc);
        adc_init();

        gpio_init_pin(&centre_console_storage->steering_storage->toggle_gpio, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);

        static InterruptSettings it_settings = {
            .priority = INTERRUPT_PRIORITY_NORMAL,
            .type = INTERRUPT_TYPE_INTERRUPT,
            .edge = INTERRUPT_EDGE_FALLING,
        };

        gpio_register_interrupt(&centre_console_storage->steering_storage->toggle_gpio, &it_settings, CC_TOGGLE_EVENT, NULL);

        return STATUS_CODE_OK;
    }

    StatusCode steering_run() {
        if (centre_console_storage == NULL) {
            return STATUS_CODE_INVALID_ARGS;
        }

        adc_run();

        uint16_t turn_data = adc_read_converted(&centre_console_storage->steering_storage->turn_signal_adc, &centre_console_storage->steering_storage->adc_readings);

        if (
            TURN_LEFT_SIGNAL_VOLTAGE_MV - VOLTAGE_TOLERANCE_MV <= turn_data && 
            turn_data <= TURN_LEFT_SIGNAL_VOLTAGE_MV + VOLTAGE_TOLERANCE_MV
        ) {
            LOG_DEBUG("LEFT\n");
            set_cc_steering_input_lights(TURN_SIGNAL_LEFT);
        } else if (
            TURN_RIGHT_SIGNAL_VOLTAGE_MV - VOLTAGE_TOLERANCE_MV <= turn_data && 
            turn_data <= TURN_RIGHT_SIGNAL_VOLTAGE_MV + VOLTAGE_TOLERANCE_MV
        ) {
            LOG_DEBUG("RIGHT\n");
            set_cc_steering_input_lights(TURN_SIGNAL_RIGHT);
        }

        uint16_t cc_data = adc_read_converted(&centre_console_storage->steering_storage->cc_adc, &centre_console_storage->steering_storage->cc_inputs);

        if (
            CRUISE_CONTROl_STALK_SPEED_DECREASE_VOLTAGE_MV - VOLTAGE_TOLERANCE_MV <= cc_data &&
            cc_data <= CRUISE_CONTROl_STALK_SPEED_DECREASE_VOLTAGE_MV + VOLTAGE_TOLERANCE_MV
        ) {
            LOG_DEBUG("CC DECREASE\n");
            set_cc_steering_input_cc(CC_DECREASE_MASK | centre_console_storage->steering_storage->cc_inputs);
        } else if (
            CRUISE_CONTROl_STALK_SPEED_INCREASE_VOLTAGE_MV - VOLTAGE_TOLERANCE_MV <= cc_data &&
            cc_data <= CRUISE_CONTROl_STALK_SPEED_INCREASE_VOLTAGE_MV + VOLTAGE_TOLERANCE_MV
        ) {
            LOG_DEBUG("CC INCREASE\n");
            set_cc_steering_input_cc(CC_INCREASE_MASK | centre_console_storage->steering_storage->cc_inputs);
        }

        return STATUS_CODE_OK;
    }

    void steering_interrupt(GpioAddress *address, GpioAddress *toggle) {
        if (address->port == toggle->port && address->pin == toggle->pin) {
            LOG_DEBUG("CC TOGGLED\n");
            set_cc_steering_input_cc(CC_TOGGLE_MASK | centre_console_storage->steering_storage->cc_inputs);
        }
    }


