#pragma once

/************************************************************************************************
 * @file   imu.h
 *
 * @brief  Header file for imu
 *
 * @date   2025-03-06
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "can.h"
#include "spi.h"

/* Intra-component Headers */
#include "bmi323.h"

/**
 * @defgroup imu
 * @brief    imu Firmware
 * @{
 */

StatusCode imu_init(Bmi323Storage *storage, Bmi323Settings *settings);

#define DELTA_T 0.1f
#define PI 3.14159265358
#define GYRO_MEAN_ERROR PI * (5.0f / 180.0f)
#define BETA sqrt(3.0f/4.0f) * GYRO_MEAN_ERROR

#include <math.h>
#include <stdio.h>

struct quaternion{
    float q1;
    float q2;
    float q3;
    float q4;
};

extern struct quaternion q_est;

struct quaternion quat_mult (struct quaternion q_L, struct quaternion q_R);
static inline void quat_scalar(struct quaternion * q, float scalar){
    q -> q1 *= scalar;
    q -> q2 *= scalar;
    q -> q3 *= scalar;
    q -> q4 *= scalar;
}
static inline void quat_add(struct quaternion * Sum, struct quaternion L, struct quaternion R){
    Sum -> q1 = L.q1 + R.q1;
    Sum -> q2 = L.q2 + R.q2;
    Sum -> q3 = L.q3 + R.q3;
    Sum -> q4 = L.q4 + R.q4;
}

static inline void quat_sub(struct quaternion * Sum, struct quaternion L, struct quaternion R){
    Sum -> q1 = L.q1 - R.q1;
    Sum -> q2 = L.q2 - R.q2;
    Sum -> q3 = L.q3 - R.q3;
    Sum -> q4 = L.q4 - R.q4;
}

static inline struct quaternion quat_conjugate(struct quaternion q){
    q.q2 = -q.q2;
    q.q3 = -q.q3;
    q.q4 = -q.q4;
    return q;
}

static inline float quat_Norm (struct quaternion q)
{
    return sqrt(q.q1*q.q1 + q.q2*q.q2 + q.q3*q.q3 +q.q4*q.q4);
}

static inline void quat_Normalization(struct quaternion * q){
    float norm = quat_Norm(*q);
    q -> q1 /= norm;
    q -> q2 /= norm;
    q -> q3 /= norm;
    q -> q4 /= norm;
}

static inline void printQuaternion (struct quaternion q){
    printf("%f %f %f %f\n", q.q1, q.q2, q.q3, q.q4);
}
void imu_filter(float ax, float ay, float az, float gx, float gy, float gz);
void eulerAngles(struct quaternion q, float* roll, float* pitch, float* yaw);

/** @} */
