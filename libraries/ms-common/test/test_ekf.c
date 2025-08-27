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

int compare_matrices(int rows, int cols, double A[rows][cols], double B[rows][cols]) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (fabs(A[i][j] - B[i][j]) > EPSILON) {
                return 0;
            }
        }
    }
    return 1;
}



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
    matrix_mult(STATE_SIZE, STATE_SIZE, STATE_SIZE, STATE_SIZE, A, B, result);
    TEST_ASSERT_TRUE(compare_matrices(3, 3, result, expected));
};


TEST_IN_TASK


void test_matrix_transpose(void){
    double A[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    double expected[3][3]={{1, 4, 7}, {2, 5, 8}, {3, 6, 9}};
    double result[3][3];
    transpose_matrix(3, 3, A, result);
    TEST_ASSERT_TRUE(compare_matrices(3, 3, result, expected));
}



/**
 * @brief Test inverting matrices
 */
/* FSM Initialization Test */

void test_matrix_inverse(void){
    double A[2][2] = {
        {-1, 3.0/2.0},
        {1,-1},
    };

    // Precomputed inverse of A (rounded)
    double expected[2][2] = {
        {2, 3},
        {2, 2}
    };

    double result[2][2];

    inverse_matrix(2, A, result);
    TEST_ASSERT_TRUE(compare_matrices(2, 2, result, expected));
}


TEST_IN_TASK
/**
 * @brief Test A matrix has correct structure
 */

void test_A_is_expected(void){
    double dt=1;
    //check for diagonals to have A[i][i]=1 and A[i][i+3]=dt;
for (int i=0;i<3;i++) {
    TEST_ASSERT_DOUBLE_WITHIN(TOL, 1.0, A[i][i]);
    TEST_ASSERT_DOUBLE_WITHIN(TOL, dt,  A[i][i+3]);
  }
  
  //Check kinematics in correct entries
  for(int i=3; i<6; i++){
    TEST_ASSERT_DOUBLE_WITHIN(TOL, 1.0, A[i][i]);
  }
  }


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
void test_update_state(void) {
    double z[MEASUREMENT_SIZE] = {1, 2, 3, 4, 5, 6};

    // Simulate measurement by copying to U
    memcpy(U, z, sizeof(z));

    // Initialize state and covariance exactly as in test
    memset(x, 0, sizeof(x));
    memset(P, 0, sizeof(P));
    memset(Q, 0, sizeof(Q));
    for (int i = 0; i < STATE_SIZE; i++) {
        P[i][i] = 1.0;
    }

    // Call the function under test
    StatusCode result = update_state();
    
    // Verify function succeeded
    if (result != STATUS_CODE_OK) {
        printf("update_state failed with status: %d\n", result);
        return;
    }

    // Debug: Print actual results
    printf("State after update:\n");
    for (int i = 0; i < STATE_SIZE; i++) {
        printf("x[%d] = %f\n", i, x[i]);
    }

    const double TOL = 1e-6;
    
    // x[0..2] should remain close to 0 (positions)
    for (int i = 0; i < 3; i++) {
        if (fabs(x[i]) > TOL) {
            printf("FAIL: x[%d] = %f, expected ~0\n", i, x[i]);
        }
    }

    // x[3..8] should be approximately 0.5 * z[0..5]
    // This happens because with R = I and P = I, the Kalman gain K ≈ 0.5*I
    for (int i = 0; i < 6; i++) {
        double expected = 0.5 * z[i];
        if (fabs(x[i + 3] - expected) > TOL) {
            printf("FAIL: x[%d] = %f, expected %f\n", i + 3, x[i + 3], expected);
        } else {
            printf("PASS: x[%d] = %f ≈ %f\n", i + 3, x[i + 3], expected);
        }
    }
}





/**
 * @brief Check Kinematics calculations
 */


/**
 * @brief Test covariance prediction
 */


/**
 * @brief test multiplying matrixes by 0
 */

void test_mult_zero(void){
     double A[STATE_SIZE][STATE_SIZE]={0};
     double B[STATE_SIZE][STATE_SIZE]={0};
     double result[STATE_SIZE][STATE_SIZE]={0};
     double expected[STATE_SIZE][STATE_SIZE]={0};
     matrix_mult(STATE_SIZE, STATE_SIZE, STATE_SIZE, STATE_SIZE, A, B, result);
     TEST_ASSERT_TRUE(compare_matrices(STATE_SIZE, STATE_SIZE, result, expected));
}