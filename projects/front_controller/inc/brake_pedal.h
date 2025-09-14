/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "front_controller.h"


typedef struct{
    uint16_t lower_value; /**< ADC reading when the pedal is considered fully released */
    uint16_t upper_value; /**< ADC reading when the pedal is considered fully pressed */
} BrakePedalCalibrationData;

typedef struct BrakePedalStorage {
  float prev_reading;

  BrakePedalCalibrationData calibration_data;
} BrakePedalStorage;

StatusCode brake_pedal_run();
StatusCode brake_pedal_init(FrontControllerStorage *storage);