/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for op_amp_api
 *
 * @date   2025-11-04
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "status.h"
#include "delay.h"
#include "opamp.h"

/* Intra-component Headers */
typedef enum {
  TEST_MODE_INVERTING_AMP = 0,
  TEST_MODE_NONINVERTING_AMP,
  TEST_MODE_PGA_AMP,
  TEST_MODE_FOLLOWER_AMP,
} TestCases; 

#define OP_AMP_TEST_MODE (TEST_MODE_PGA_AMP) //change this for each test

TASK(op_amp_api, TASK_STACK_1024) {
  while (true) {
    LOG_DEBUG("Op-Amp smoke test running...");
    delay_ms(1000);
  }
}

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
int main(int argc, char *argv[]) {
  mpxe_init(argc, argv);
#else
int main() {

  mcu_init();
  tasks_init();
  log_init();
#endif

  StatusCode status = opamp_init();
  
  if (status != STATUS_CODE_OK) {
    LOG_ERROR("OPAMP init failed: %d", status);
    return 1; //1 if error
  }

  OpampConfig config;

  #if (OP_AMP_TEST_MODE == TEST_MODE_INVERTING_AMP)
    config.vinp_sel = OPAMP_NONINVERTING_IO0;
    config.vinm_sel = OPAMP_INVERTING_IO0;
    config.pga_gain =  OPAMP_PROGRAMMABLE_GAIN_2;
    config.output_to_adc = false;

  #elif (OP_AMP_TEST_MODE == TEST_MODE_NONINVERTING_AMP)
    config.vinp_sel = OPAMP_NONINVERTING_DAC;
    config.vinm_sel = OPAMP_INVERTING_PGA;
    config.pga_gain = OPAMP_PROGRAMMABLE_GAIN_4;
    config.output_to_adc = true;

  #elif (OP_AMP_TEST_MODE == TEST_MODE_PGA_AMP)
    config.vinp_sel = OPAMP_NONINVERTING_IO0;
    config.vinm_sel = OPAMP_INVERTING_PGA;
    config.pga_gain = OPAMP_PROGRAMMABLE_GAIN_8;
    config.output_to_adc = true;

  #elif (OP_AMP_TEST_MODE == TEST_MODE_FOLLOWER_AMP)
    config.vinp_sel = OPAMP_NONINVERTING_IO0;
    config.vinm_sel = OPAMP_INVERTING_FOLLOWER;
    config.pga_gain = OPAMP_PROGRAMMABLE_GAIN_16;
    config.output_to_adc = false;

  #endif

  status = opamp_configure(OPAMP_1, &config);
  if (status != STATUS_CODE_OK) {
    LOG_ERROR("OPAMP configure failed: %d", status);
    return 1; //1 if error
  }

  status = opamp_start(OPAMP_1);
  if (status != STATUS_CODE_OK) {
    LOG_ERROR("OPAMP start failed: %d", status);
    return 1; //1 if error
  }

  tasks_init_task(op_amp_api, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}