
/************************************************************************************************
 * @file   log.c
 *
 * @brief  Source code for the Extended Kalman Filter for the BMI323
 *
 * @date   2024-11-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdlib.h>
#include <math.h>
/* Inter-component Headers */
#include "ekf.h"
#include "status.h"
/* Intra-component Headers */
#include "log.h"

/**
*@brief Maximum acceptable error
*/
#define EPSILON 1e-9

 /** @brief State transition matrix (Intialize diagonals with time derivatives of kinematic equations)*/ 
 double A[STATE_SIZE][STATE_SIZE] = {
    {1, 0, 0, DT, 0, 0,  0,  0,  0},
    {0, 1, 0, 0, DT, 0,  0,  0,  0},
    {0, 0, 1, 0, 0, DT,  0,  0,  0},
    {0, 0, 0, 1, 0, 0,   0,  0,  0},
    {0, 0, 0, 0, 1, 0,   0,  0,  0},
    {0, 0, 0, 0, 0, 1,   0,  0,  0},
    {0, 0, 0, 0, 0, 0,   1, DT, 0},
    {0, 0, 0, 0, 0, 0,   0, 1, DT},
    {0, 0, 0, 0, 0, 0,   0, 0, 1}
};

double x[STATE_SIZE] = {0};
double P[STATE_SIZE][STATE_SIZE] = {0};
double Q[STATE_SIZE][STATE_SIZE] = {0};
double R[MEASUREMENT_SIZE][MEASUREMENT_SIZE] = {
    {1, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0},
    {0, 0, 1, 0, 0, 0},
    {0, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 1, 0},
    {0, 0, 0, 0, 0, 1},
};
double U[MEASUREMENT_SIZE] = {0};



/**
*@brief Takes in 2 matrices and performs matrix multiplication, of any size
*/
StatusCode matrix_mult(int rows_A, int cols_A, int rows_B,int cols_B, double A[rows_A][cols_A], double B[rows_B][cols_B], double result[rows_A][cols_B]) {
    if (!A||!B||!result) return STATUS_CODE_INVALID_ARGS;
    
    for (int i = 0; i < rows_A; i++) {
        for (int j = 0; j < cols_B; j++) {
            result[i][j] = 0.0;
            for (int k = 0; k < cols_A; k++) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return STATUS_CODE_OK;
}

/**
*@brief Takes in matrix and outputs its tranpose
*/
StatusCode transpose_matrix(int rows, int cols, double matrix[rows][cols], double transpose[cols][rows]){
    if (!matrix||!transpose) return STATUS_CODE_INVALID_ARGS;
    
    for (int i=0; i<rows; i++){
        for (int j=0; j<cols; j++){
            transpose[j][i]=matrix[i][j];
        }
    }

    return STATUS_CODE_OK;
}


/**
*@brief Takes in matrix and outputs its inverse
 */
StatusCode inverse_matrix(int size, double input[size][size], double output[size][size]) {
    if (!input || !output || size <= 0) return STATUS_CODE_INVALID_ARGS;

    double temp[size][2 * size];

    // Initialize output to identity matrix
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            output[i][j] = (i == j) ? 1.0 : 0.0;
            temp[i][j] = input[i][j];
            temp[i][j + size] = output[i][j];
        }
    }

    // Gauss-Jordan elimination
    for (int i = 0; i < size; i++) {
        double pivot = temp[i][i];
        if (fabs(pivot) < EPSILON) return STATUS_CODE_INVALID_ARGS;

        for (int j = 0; j < 2 * size; j++) {
            temp[i][j] /= pivot;
        }

        for (int k = 0; k < size; k++) {
            if (k == i) continue;
            double factor = temp[k][i];
            for (int j = 0; j < 2 * size; j++) {
                temp[k][j] -= factor * temp[i][j];
            }
        }
    }

    // Extract inverse from augmented matrix
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            output[i][j] = temp[i][j + size];
        }
    }

    return STATUS_CODE_OK;
}


//use kinematics eqns to predict state
StatusCode predict_state(void){
    for (int i=0; i<3; i++){
        x[i]+=x[i+3]+0.5*U[i]*DT*DT;
        x[i+3]+=U[i]*DT;
        // Using equation X(t)=V(t)*DT
    }
    
    for (int i=6; i<STATE_SIZE; i++){
        x[i]+=U[i-3]*DT;
    }
    return STATUS_CODE_OK;
}



/**
*@brief Update covraiance from current transition matrix
*/
StatusCode predict_covariance(void) {
    double A_T[STATE_SIZE][STATE_SIZE]={};
    double temp[STATE_SIZE][STATE_SIZE]={};
    double P_curr[STATE_SIZE][STATE_SIZE] = {0};

    transpose_matrix(STATE_SIZE, STATE_SIZE, A, A_T);
    matrix_mult(STATE_SIZE, STATE_SIZE, STATE_SIZE, STATE_SIZE, A, P, P_curr);
    matrix_mult(STATE_SIZE, STATE_SIZE, STATE_SIZE, STATE_SIZE, P_curr, A_T, P);


    // Add process noise Q
    for (int i = 0; i < STATE_SIZE; i++) {
        for (int j = 0; j < STATE_SIZE; j++) {
            P[i][j] += Q[i][j];
        }
    }
    return STATUS_CODE_OK;
}

/**
*@brief Update state using measurement and kalman gain
*/
StatusCode update_state(void) {
   double H[MEASUREMENT_SIZE][STATE_SIZE] = {0}; 
    double K[STATE_SIZE][MEASUREMENT_SIZE] = {0}; 
    double S[MEASUREMENT_SIZE][MEASUREMENT_SIZE] = {0};
    double S_inv[MEASUREMENT_SIZE][MEASUREMENT_SIZE] = {0};
    double H_T[STATE_SIZE][MEASUREMENT_SIZE] = {0};
    double y[MEASUREMENT_SIZE] = {0};

    // Initialize H matrix (measurement model: z = H*x + noise)
    for (int i = 0; i < MEASUREMENT_SIZE; i++) {
        H[i][i + 3] = 1.0;  // Direct observation of velocities/angular rates
    }

    //innovation: y = z - H*x
    for (int i = 0; i < MEASUREMENT_SIZE; i++) {
        y[i] = U[i]; // measurement
        for (int j = 0; j < STATE_SIZE; j++) {
            y[i] -= H[i][j] * x[j];
        }
    }

    // Transpose H: H_T is STATE_SIZE x MEASUREMENT_SIZE
    transpose_matrix(MEASUREMENT_SIZE, STATE_SIZE, H, H_T);

    //  1: H*P -> temp1 (MEASUREMENT_SIZE x STATE_SIZE)
    double temp1[MEASUREMENT_SIZE][STATE_SIZE] = {0};
    matrix_mult(MEASUREMENT_SIZE, STATE_SIZE, STATE_SIZE, STATE_SIZE, H, P, temp1);
    
    //  2: (H*P)*H^T -> S (MEASUREMENT_SIZE x MEASUREMENT_SIZE)  
    matrix_mult(MEASUREMENT_SIZE, STATE_SIZE, STATE_SIZE, MEASUREMENT_SIZE, temp1, H_T, S);

  
    for (int i = 0; i < MEASUREMENT_SIZE; i++) {
        for (int j = 0; j < MEASUREMENT_SIZE; j++) {
            S[i][j] += R[i][j];
        }
    }

    if (inverse_matrix(MEASUREMENT_SIZE, S, S_inv) != STATUS_CODE_OK) {
        return STATUS_CODE_INVALID_ARGS;
    }

    // Compute Kalman gain: K = P*H^T*S^(-1)
    // Step 1: P*H^T -> temp2 (STATE_SIZE x MEASUREMENT_SIZE)
    double temp2[STATE_SIZE][MEASUREMENT_SIZE] = {0};
    matrix_mult(STATE_SIZE, STATE_SIZE, STATE_SIZE, MEASUREMENT_SIZE, P, H_T, temp2);
    
    // Step 2: (P*H^T)*S^(-1) -> K (STATE_SIZE x MEASUREMENT_SIZE)
    matrix_mult(STATE_SIZE, MEASUREMENT_SIZE, MEASUREMENT_SIZE, MEASUREMENT_SIZE, temp2, S_inv, K);

    // Update state estimate: x = x + K*y
    for (int i = 0; i < STATE_SIZE; i++) {
        for (int j = 0; j < MEASUREMENT_SIZE; j++) {
            x[i] += K[i][j] * y[j];
        }
    }

    // Update covariance: P = (I - K*H)*P
    double I_KH[STATE_SIZE][STATE_SIZE] = {0};
    double KH[STATE_SIZE][STATE_SIZE] = {0};

    matrix_mult(STATE_SIZE, MEASUREMENT_SIZE, MEASUREMENT_SIZE, STATE_SIZE, K, H, KH);

    // Create (I - K*H)
    for (int i = 0; i < STATE_SIZE; i++) {
        for (int j = 0; j < STATE_SIZE; j++) {
            I_KH[i][j] = (i == j ? 1.0 : 0.0) - KH[i][j];
        }
    }

    double P_temp[STATE_SIZE][STATE_SIZE] = {0};
    matrix_mult(STATE_SIZE, STATE_SIZE, STATE_SIZE, STATE_SIZE, I_KH, P, P_temp);
    
    // Copy results back to P
    for (int i = 0; i < STATE_SIZE; i++) {
        for (int j = 0; j < STATE_SIZE; j++) {
            P[i][j] = P_temp[i][j];
        }
    }

    return STATUS_CODE_OK;


}




