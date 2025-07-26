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
#include <
/* Inter-component Headers */

/* Intra-component Headers */
#include "semaphore.h"


BmsStorage *bms;
static StateOfChargStorage s_storage;

static float soc_estimate =0.0f;
static float soc_covraince=1.0f;

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


static float predict_voltage(void);

static float soc_to_ocv(float ocv);

static float h_prime(float soc);

//StatusCode soc_ekf_update(flaot dt_hr){;
