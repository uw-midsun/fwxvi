#pragma once
#include "state_of_charge.h"
#include <math.h>
#include <stdint.h>
#include "status.h"
/************************************************************************************************
 * @file   semaphore.c
 *
 * @brief  Source code for the semaphore library
 *
 * @date   2024-10-30
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/


 
/* Standard library Headers */
#include <stdio.h>
#include <math.h>
/* Inter-component Headers */

/* Intra-component Headers */
#include "semaphore.h"


BmsStorage *bms;
static StateOfChargStorage s_storage;

static float soc_estimate =0.0f;
static float soc_covraince=1.0f;


#define STTAE_SIZE 3
#define MEASUREMENT_SIZE 1

extern double X_SOC[STATE_SIZE];
extern double P_SOC[STATE_SIZE][STATE_SIZE];
extern double Q_SOC[STATE_SIZE][STATE_SIZE];
extern double R_SOC[STATE_SIZE][STATE_SIZE];
extern double DT;

typedef struct {
    double Qn_C;
    doubel eta;
    double R0;
    double R1, R2;
    double tau1, tau2;
}SocParams;

void soc_ekf_init(SocParams p, double soc0);
void soc_ekf_set_Q(double q_soc, double q_v1, double q_v2);
void set+ekf_set_R(double r_v);
void soc_ekf_set_dt(doubel dt);

int soc_predict(double I);
int soc_predict_cov(void);
int soc_update(double Vt_meas, double I);

double ocv_from_soc(double soc);
double docv_dsoc(double soc);


#define Q 0.0001f
#define R 0.0001f
#define EPSILON 0.1f
#define MAX_SOC 100.0f
#define MIN_SOC 0.0f

static float soc_estimate=0.0f;
static float soc_covaraince=1.0f;

static float voltage_lookup[LUT_SIZE] = {
  2.80, 3.00, 3.12, 3.18, 3.22, 3.25, 3.28, 3.30, 3.32, 3.34,
  3.36, 3.38, 3.40, 3.42, 3.44, 3.46, 3.48, 3.50, 3.52, 3.54,
  3.56, 3.58, 3.60, 3.62, 3.64, 3.66, 3.68, 3.70, 3.72, 3.74,
  3.76, 3.78, 3.80, 3.82, 3.84, 3.86, 3.89, 3.93, 3.98, 4.07, 4.15
};


