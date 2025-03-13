#pragma once

#include "adc.h"
#include "cc_hw_defs.h"
#include "gpio.h"
#include "gpio.h"
#include "centre_console.h"
#include "interrupts.h"
#include "gpio_interrupts.h"
#include "centre_console_setters.h"


#define VOLTAGE_TOLERANCE_MV 100

#define TURN_LEFT_SIGNAL_VOLTAGE_MV 2100
#define TURN_RIGHT_SIGNAL_VOLTAGE_MV 1200
#define NEUTRAL_SIGNAL_VOLTAGE_MV 3300

#define CRUISE_CONTROl_STALK_SPEED_INCREASE_VOLTAGE_MV 1600
#define CRUISE_CONTROl_STALK_SPEED_DECREASE_VOLTAGE_MV 775
#define CRUISE_CONTROl_STALK_NEUTRAL_VOLTAGE_MV 3300

#define CC_TOGGLE_EVENT 0x0

struct SteeringStorage {
    uint16_t adc_readings;
    uint16_t cc_inputs;
    GpioAddress turn_signal_adc;
    GpioAddress cc_adc;
    GpioAddress toggle_gpio;
};

typedef enum TurnSignalState {
    TURN_SIGNAL_OFF = 0,
    TURN_SIGNAL_LEFT = 1,
    TURN_SIGNAL_RIGHT = 2,
  } TurnSignalState;
  
  typedef enum {
    CC_DECREASE_MASK = 0x01,
    CC_INCREASE_MASK = 0x02,
    CC_TOGGLE_MASK = 0x04,
  } CruiseControlMask;


//initializes ADCS for reading steering inputs
StatusCode steering_init(CentreConsoleStorage *storage);

// read the input ADCs and determine the signal is left / right or increasing / decreasing cruise control

StatusCode steering_run();

void steering_interrupt(GpioAddress *address, GpioAddress *toggle);
