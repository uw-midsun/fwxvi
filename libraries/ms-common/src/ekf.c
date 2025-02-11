
#include <stdlib.h>
#include "ekf.h"
#include "log.h"


void matrix_mult(double A[][STATE_SIZE], double B[][STATE_SIZE], double result[][STATE_SIZE]) {
    for (int i = 0; i < STATE_SIZE; i++) {
        for (int j = 0; j < STATE_SIZE; j++) {
            result[i][j] = 0;
            for (int k = 0; k < STATE_SIZE; k++) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void matrix_transpose(double matrix[][STATE_SIZE], double transpose[][STATE_SIZE]){
    for (int i=0; i<STATE_SIZE; i++){
        for (int j=0; j<STATE_SIZE; j++){
            transpose[i][j]=matrix[j][i];
        }
    }
}
//use kinematics eqns to predict state
void predict_state(){

    for (int i=0; i<3; i++){
        x[i]+=x[i+3]+0.5*U[i]*DT*DT;
        x[i+3]+=U[i]*DT;
    }
    for (int i=6; i<STATE_SIZE; i++){
        x[i]+=U[i-3]*DT;
    }
}

//compute P K|K-1
void predict_covariance() {
    double A_T[STATE_SIZE][STATE_SIZE]={};
    double temp[STATE_SIZE][STATE_SIZE]={};
    double P_curr[STATE_SIZE][STATE_SIZE] = {0};
    matrix_transpose(A, A_T);
    matrix_mult(A, P, P_curr);
    matrix_mult(P_curr, A_T, P);
    // Add process noise Q
    for (int i = 0; i < STATE_SIZE; i++) {
        for (int j = 0; j < STATE_SIZE; j++) {
            P[i][j] += Q[i][j];
        }
    }
}

void update_state() {
    double H[MEASUREMENT_SIZE][STATE_SIZE] = {0}; // map state vector to measurements
    double K[STATE_SIZE][MEASUREMENT_SIZE] = {0}; //kalman gain 
    double S[MEASUREMENT_SIZE][MEASUREMENT_SIZE] = {0};//uncertainty in residual
    double S_inv[MEASUREMENT_SIZE][MEASUREMENT_SIZE] = {0};
    double H_T[STATE_SIZE][MEASUREMENT_SIZE] = {0};
    double P_H_T[STATE_SIZE][MEASUREMENT_SIZE] = {0};
    double y[MEASUREMENT_SIZE] = {0}; 

    // initialize H (maps state to measurement)
    for (int i = 0; i < MEASUREMENT_SIZE; i++) {
        H[i][i + 3] = 1.0;
    }

    // y~ = z - H * x
    for (int i = 0; i < MEASUREMENT_SIZE; i++) {
        y[i] = U[i] - (H[i][3] * x[3] + H[i][4] * x[4] + H[i][5] * x[5]);
    }

    // compute S|k = H * P K|K-1 * H^T + R
    matrix_transpose(H, H_T);
    matrix_mult(H, P, P_H_T);
    matrix_mult(P_H_T, H_T, S);

    for (int i = 0; i < MEASUREMENT_SIZE; i++) {
        for (int j = 0; j < MEASUREMENT_SIZE; j++) {
            S[i][j] += R[i][j];
        }
    }

    inverse_matrix(S, S_inv);

    matrix_mult(P_H_T, S_inv, K);

    //  change state to x = x + K * y~
    for (int i = 0; i < STATE_SIZE; i++) {
        for (int j = 0; j < MEASUREMENT_SIZE; j++) {
            x[i] += K[i][j] * y[j];
        }
    }

    // Update P K|K = (I - K * H) * P K|K-1
    double I_KH[STATE_SIZE][STATE_SIZE] = {0};
    double KH[STATE_SIZE][STATE_SIZE] = {0};

    matrix_mult(K, H, KH);

    // create P K|K
    for (int i = 0; i < STATE_SIZE; i++) {
        for (int j = 0; j < STATE_SIZE; j++) {
            I_KH[i][j] = (i == j ? 1 : 0) - KH[i][j];
            //create an identity matrix and then sutract the KH matrix
        }
    }

    double P_temp[STATE_SIZE][STATE_SIZE] = {0};
    matrix_mult(I_KH, P, P_temp);

    for (int i = 0; i < STATE_SIZE; i++) {
        for (int j = 0; j < STATE_SIZE; j++) {
            P[i][j] = P_temp[i][j];
        }
    }
}


int main(){

    predict_state();
    predict_covariance();
    update_state();
    LOG_DEBUG("Position; (%f, %f, %f)\n", x[0], x[1], x[2]);
    LOG_DEBUG("Speed; (%f, %f, %f)\n", x[3], x[4], x[5]);
    LOG_DEBUG("Angle; (%f, %f, %f)\n", x[6], x[7], x[8]);

}