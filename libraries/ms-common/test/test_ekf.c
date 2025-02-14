/************************************************************************************************
 * @file   test_ekf.c
 *
 * @brief  Unit tests for the Extended Kalman Filter (EKF) implementation
 *
 * @date   2025-02-11
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard Library Headers */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* Inter-component Headers */
#include "ekf.h"
#include "test_helpers.h"
#include "unity.h"
#define EPSILON 1e-12 

static int is_equal(double matrix1[][STATE_SIZE]){
    for (int i = 0; i < STATE_SIZE; i++) {
        for (int j = 0; j < STATE_SIZE; j++) {
            if (fabs(A[i][j] - B[i][j]) > EPSILON) {
                return 0;  
            }
        }
    }
    return 1;
};

/* Setup & Teardown */
void setup_test(void) {
  memset(x, 0 , sizeof(x));
  memset(P, 0, sizeof(P));
  memset(Q, 0, sizeof(Q));
  memset(U, 0, sizeof(U));
}
void teardown_test(void) {}

TEST_IN_TASK

void test_matrix_mult(void){
    double A[STATE_SIZE][STATE_SIZE] = {{1, 2}, {3, 4}};
    double B[STATE_SIZE][STATE_SIZE] = {{5, 6}, {7, 8}};
    double expected[STATE_SIZE][STATE_SIZE] = {{19, 22}, {43, 50}};
    double result[STATE_SIZE][STATE_SIZE] = {0};
    multiply_matrices(A, B, result);
    TEST_ASSERT_TRUE(compare_matrices(result, expected));
};

TEST_IN_TASK

void test_matrix_transpose(void){
    double A[STATE_SIZE][STATE_SIZE] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    double expected[STATE_SIZE][STATE_SIZE];
    double result{STATE_SIZE}[STATE_SIZE];
    transponse_matrix(A, expected);
    TEST_ASSERT_TRUE(compare_matrices(result, expected));
}

/* FSM Initialization Test */

TEST_IN_TASK
void test_predict_state(void) {
    // Initialize state vector
    x[0] = 0; x[1] = 0; x[2] = 0;
    x[3] = 1; x[4] = 1; x[5] = 1;
    U[0] = 1; U[1] = 2; U[2] = 3;

    predict_state();

    // Expected results after one second
    TEST_ASSERT_FLOAT_WITHIN(EPSILON, 1 + 0.5 * 1 * DT * DT, x[0]);
    TEST_ASSERT_FLOAT_WITHIN(EPSILON, 1 + 0.5 * 2 * DT * DT, x[1]);
    TEST_ASSERT_FLOAT_WITHIN(EPSILON, 1 + 0.5 * 3 * DT * DT, x[2]);
}


/* Check covariance prediction works */
TEST_IN_TASK
void test_predict_covariance(void) {
    for (int i = 0; i < STATE_SIZE; i++) {
        for (int j = 0; j < STATE_SIZE; j++) {
            A[i][j] = (i == j) ? 1.0 : 0.0;
        }
    }

    for (int i = 0; i < STATE_SIZE; i++) {
        P[i][i] = 1.0;
    }
    predict_covariance();

    for (int i = 0; i < STATE_SIZE; i++) {
        TEST_ASSERT_EQUAL(1.0 + Q[i][i], P[i][i]);
    }

}

/* Valid FSM State Transition */
TEST_IN_TASK
void test_update_state(void){
    double z[MEASUREMENT_SIZE]={1,2,3,4,5,6};
    double H[MEASUREMENT_SIZE][STATE_SIZE] = { 
        {1, 0, 0, 0, 0, 0, 0, 0, 0},  // Position x
        {0, 1, 0, 0, 0, 0, 0, 0, 0},  // Position y
        {0, 0, 1, 0, 0, 0, 0, 0, 0},  // Position z
        {0, 0, 0, 1, 0, 0, 0, 0, 0},  // Velocity x
        {0, 0, 0, 0, 1, 0, 0, 0, 0},  // Velocity y
        {0, 0, 0, 0, 0, 1, 0, 0, 0}   // Velocity z
    };
    double expected_x[STATE_SIZE] = {1, 2, 3, 0.1, 0.2, 0.3, 0, 0, 0};

    // Initialize state x and covariance P
    memset(x, 0, sizeof(x));  
    for (int i = 0; i < STATE_SIZE; i++) {
        P[i][i] = 1.0; 

    update_state(z, H);
    for (int i = 0; i < MEASUREMENT_SIZE; i++) {
        TEST_ASSERT_FLOAT_WITHIN(EPSILON, expected_x[i], x[i]);
    }

}


void generate_fake_data(){
    *accel_x = (rand() % 4000 - 2000) / 100.0f;
    *accel_y = (rand() % 4000 - 2000) / 100.0f;
    *accel_z = (rand() % 4000 - 2000) / 100.0f;
    *omega = (rand() % 2500 - 1250) / 100.0f;

}

typedef struct {
    float accel_x, accel_y, accel_z,;
    float omega;
} IMU_Data;


TEST_IN_TASK

void test_update_imu_data(float ){
    IMU_Data imu_data;
    generate_fake_imu_data(&imu_data);
    float expected_x[STATE_SIZE] = {imu_data.accel_x, imu_data.accel_y, imu_data.accel_z, imu_data.omega};
    update_state(expected_x);

}