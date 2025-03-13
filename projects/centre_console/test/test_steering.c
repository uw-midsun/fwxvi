
#include "test_helpers.h"
#include "adc.h"
#include "steering.h"
#include "log.h"

#define VOLTAGE_TOLERANCE_MV 100

#define TURN_LEFT_SIGNAL_VOLTAGE_MV 2100
#define TURN_RIGHT_SIGNAL_VOLTAGE_MV 1200
#define NEUTRAL_SIGNAL_VOLTAGE_MV 3300

#define CRUISE_CONTROl_STALK_SPEED_INCREASE_VOLTAGE_MV 1600
#define CRUISE_CONTROl_STALK_SPEED_DECREASE_VOLTAGE_MV 775
#define CRUISE_CONTROl_STALK_NEUTRAL_VOLTAGE_MV 3300

static CentreConsoleStorage s_storage;

void setup_test(void) {
    steering_init(&s_storage);
}

TEST_IN_TASK
void test_steering_turn_left(void) {
    adc_set_reading(&s_storage.steering_storage->turn_signal_adc, TURN_LEFT_SIGNAL_VOLTAGE_MV);
    steering_run();

    TEST_ASSERT_EQUAL(STATUS_CODE_OK, steering_run());
}

TEST_IN_TASK
void test_steering_turn_right(void) {
    adc_set_reading(&s_storage.steering_storage->turn_signal_adc, TURN_RIGHT_SIGNAL_VOLTAGE_MV);
    steering_run();

    TEST_ASSERT_EQUAL(STATUS_CODE_OK, steering_run());
}


TEST_IN_TASK
void no_turn(void) {
    adc_set_reading(&s_storage.steering_storage->turn_signal_adc, NEUTRAL_SIGNAL_VOLTAGE_MV);
    steering_run();

    TEST_ASSERT_EQUAL(STATUS_CODE_OK, steering_run());

}

TEST_IN_TASK
void cc_inc_voltage(void) {
    adc_set_reading(&s_storage.steering_storage->cc_adc, CRUISE_CONTROl_STALK_SPEED_INCREASE_VOLTAGE_MV);
    steering_run();

    TEST_ASSERT_EQUAL(STATUS_CODE_OK, steering_run());
}

TEST_IN_TASK
void cc_dec_voltage(void) {
    adc_set_reading(&s_storage.steering_storage->cc_adc, CRUISE_CONTROl_STALK_SPEED_DECREASE_VOLTAGE_MV);
    steering_run();

    TEST_ASSERT_EQUAL(STATUS_CODE_OK, steering_run());
}

TEST_IN_TASK
void cc_same_voltage(void) {
    adc_set_reading(&s_storage.steering_storage->cc_adc, CRUISE_CONTROl_STALK_NEUTRAL_VOLTAGE_MV);
    steering_run();

    TEST_ASSERT_EQUAL(STATUS_CODE_OK, steering_run());
}

