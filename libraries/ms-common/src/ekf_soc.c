#include "state_of_charge.h"
#include "ekf.h"
#include "ekf_soc.h"
#include <math.h>
#define H_epsilon 0.1f
/************************************************************************************************
 * @file   ekf_soc.c
 *
 * @brief  Source code for the ekf_soc library
 *
 * @date   2024-10-30
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/


BmsStorage *bms_charge;
state_of_charge_init(bms_charge);

StatusCode ekf_soc_init(BmsStorage *bms_storage);
StatusCode ekf_soc_step(float dt_seconds);
float ekf_get_soc_fraction(void);
float ekf_get_vrc(void);

#define STATE_SIZE 2
//Only need to measure terminal voltage
#define MEASUEREMENT_SIZE 1



double A[STATE_SIZE][STATE_SIZE];
double Q[STATE_SIZE][STATE_SIZE];
double R[MEASUEREMENT_SIZE][MEASUEREMENT_SIZE];
double P[STTAE_SIZE][STATE_SIZE];
double x[STATE_SIZE];
double H[MEASUEREMENT_SIZE][MEASUEREMENT_SIZE];

double ocv_lookup(double soc);
StatusCode ekf_predict(double I, double dt, double C_nom, double R_int)

void ekf_set_