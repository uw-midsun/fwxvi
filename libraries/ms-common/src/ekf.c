#include <stdlib.h>
#include <math.h>
#include "ekf.h"
#include "log.h"

// Global variable definitions
float x[STATE_SIZE] = {0};
float P[STATE_SIZE][STATE_SIZE] = {0};
float U[MEASUREMENT_SIZE] = {0};
float Q[STATE_SIZE][STATE_SIZE] = {0};

// Measurement noise covariance matrix
float R[MEASUREMENT_SIZE][MEASUREMENT_SIZE] = {
    {1, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0},
    {0, 0, 1, 0, 0, 0},
    {0, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 1, 0},
    {0, 0, 0, 0, 0, 1}
};

// State transition matrix
float A[STATE_SIZE][STATE_SIZE] = {
    {1, 0, 0, DT, 0,  0,  0,  0,  0},
    {0, 1, 0, 0,  DT, 0,  0,  0,  0},
    {0, 0, 1, 0,  0,  DT, 0,  0,  0},
    {0, 0, 0, 1,  0,  0,  0,  0,  0},
    {0, 0, 0, 0,  1,  0,  0,  0,  0},
    {0, 0, 0, 0,  0,  1,  0,  0,  0},
    {0, 0, 0, 0,  0,  0,  1,  DT, 0},
    {0, 0, 0, 0,  0,  0,  0,  1,  DT},
    {0, 0, 0, 0,  0,  0,  0,  0,  1}
};

float B[MEASUREMENT_SIZE][MEASUREMENT_SIZE] = {0};

void matrix_mult(int row1, int col1, float first[][col1],
                int row2, int col2, float second[][col2],
                float result[][col2]) {
    for (int i = 0; i < row1; i++) {
        for (int j = 0; j < col2; j++) {
            result[i][j] = 0;
            for (int k = 0; k < col1; k++) {
                result[i][j] += first[i][k] * second[k][j];
            }
        }
    }
}

void matrix_transpose(float matrix[][STATE_SIZE],
                     float transpose[][STATE_SIZE]) {
    for (int i = 0; i < STATE_SIZE; i++) {
        for (int j = 0; j < STATE_SIZE; j++) {
            transpose[i][j] = matrix[j][i];
        }
    }
}

void cofactor(float matrix[][STATE_SIZE],
             float temp[][STATE_SIZE],
             int p, int q, int n) {
    int i = 0, j = 0;
    
    for (int row = 0; row < n; row++) {
        for (int col = 0; col < n; col++) {
            if (row != p && col != q) {
                temp[i][j++] = matrix[row][col];
                if (j == n - 1) {
                    j = 0;
                    i++;
                }
            }
        }
    }
}

float determinant(float matrix[][STATE_SIZE], int n) {
    if (n == 1) {
        return matrix[0][0];
    }

    float det = 0;
    int sign = 1;
    float temp[STATE_SIZE][STATE_SIZE] = {0};

    for (int i = 0; i < n; i++) {
        cofactor(matrix, temp, 0, i, n);
        det += sign * matrix[0][i] * determinant(temp, n - 1);
        sign = -sign;
    }

    return det;
}

void adjoint(float matrix[][STATE_SIZE],
            float adj[][STATE_SIZE],
            int n) {
    if (n == 1) {
        adj[0][0] = 1;
        return;
    }

    int sign;
    float temp[STATE_SIZE][STATE_SIZE] = {0};

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            cofactor(matrix, temp, i, j, n);
            sign = ((i + j) % 2 == 0) ? 1 : -1;
            adj[j][i] = sign * determinant(temp, n - 1);
        }
    }
}

int inverse(float matrix[][STATE_SIZE],
           float inverse[][STATE_SIZE],
           int n) {
    float det = determinant(matrix, n);

    if (fabs(det) < 1e-10) {
        LOG_DEBUG("Inverse does not exist\n");
        return 0;
    }

    float adj[STATE_SIZE][STATE_SIZE] = {0};
    adjoint(matrix, adj, n);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            inverse[i][j] = adj[i][j] / det;
        }
    }

    return 1;
}

void predict_state(x[STATE_SIZE]) {
    // Update position and velocity using velocity and acceleration
    for (int i = 0; i < 3; i++) {
        x[i] += x[i+3] * DT + 0.5 * U[i] * DT * DT;
        x[i+3] += U[i] * DT;
    }
    
    // Update orientation
    for (int i = 6; i < STATE_SIZE; i++) {
        x[i] += U[i-3] * DT;
    }
}

void predict_covariance(void) {
    float A_T[STATE_SIZE][STATE_SIZE] = {0};
    float temp[STATE_SIZE][STATE_SIZE] = {0};
    float P_curr[STATE_SIZE][STATE_SIZE] = {0};
    
    matrix_transpose(A, A_T);
    matrix_mult(STATE_SIZE, STATE_SIZE, A, 
               STATE_SIZE, STATE_SIZE, P, P_curr);
    matrix_mult(STATE_SIZE, STATE_SIZE, P_curr,
               STATE_SIZE, STATE_SIZE, A_T, P);
    
    // Add process noise 
    for (int i = 0; i < STATE_SIZE; i++) {
        for (int j = 0; j < STATE_SIZE; j++) {
            P[i][j] += Q[i][j];
        }
    }
}

void update_state(float z[MEASUREMENT_SIZE]) {
    float H[MEASUREMENT_SIZE][STATE_SIZE] = {0};
    float K[STATE_SIZE][MEASUREMENT_SIZE] = {0};
    float S[MEASUREMENT_SIZE][MEASUREMENT_SIZE] = {0};
    float S_inv[MEASUREMENT_SIZE][MEASUREMENT_SIZE] = {0};
    float H_T[STATE_SIZE][MEASUREMENT_SIZE] = {0};
    float P_H_T[STATE_SIZE][MEASUREMENT_SIZE] = {0};
    float y[MEASUREMENT_SIZE] = {0};

    // Set up measurement matrix
    for (int i = 0; i < 3; i++) {
        H[i][i] = 1.0f;
        H[i + 3][i + 3] = 1.0f;
    }

    // Calculate innovation
    for (int i = 0; i < MEASUREMENT_SIZE; i++) {
        float predicted_measurement = 0;
        for (int j = 0; j < STATE_SIZE; j++) {
            predicted_measurement += H[i][j] * x[j];
        }
        y[i] = x[i] - predicted_measurement;
    }

    matrix_transpose(H, H_T);
    matrix_mult(STATE_SIZE, STATE_SIZE, P,
               MEASUREMENT_SIZE, STATE_SIZE, H_T, P_H_T);
    matrix_mult(MEASUREMENT_SIZE, STATE_SIZE, H,
               STATE_SIZE, MEASUREMENT_SIZE, P_H_T, S);

    // Add measurement noise
    for (int i = 0; i < MEASUREMENT_SIZE; i++) {
        for (int j = 0; j < MEASUREMENT_SIZE; j++) {
            S[i][j] += R[i][j];
        }
    }

    if (!inverse(S, S_inv, MEASUREMENT_SIZE)) {
        LOG_DEBUG("Failed to compute S inverse\n");
        return;
    }

    matrix_mult(STATE_SIZE, MEASUREMENT_SIZE, P_H_T,
               MEASUREMENT_SIZE, MEASUREMENT_SIZE, S_inv, K);

    // Update state estimate
    for (int i = 0; i < STATE_SIZE; i++) {
        for (int j = 0; j < MEASUREMENT_SIZE; j++) {
            x[i] += K[i][j] * y[j];
        }
    }

    // Update covariance matrix
    float I_KH[STATE_SIZE][STATE_SIZE] = {0};
    float KH[STATE_SIZE][STATE_SIZE] = {0};
    
    matrix_mult(STATE_SIZE, MEASUREMENT_SIZE, K,
               MEASUREMENT_SIZE, STATE_SIZE, H, KH);

    for (int i = 0; i < STATE_SIZE; i++) {
        for (int j = 0; j < STATE_SIZE; j++) {
            I_KH[i][j] = (i == j ? 1.0f : 0.0f) - KH[i][j];
        }
    }

    float P_temp[STATE_SIZE][STATE_SIZE] = {0};
    //should be udated with pk|k-1
    matrix_mult(STATE_SIZE, STATE_SIZE, I_KH,
               STATE_SIZE, STATE_SIZE, P, P_temp);
    
    // Copy updated covariance matrix
    for (int i = 0; i < STATE_SIZE; i++) {
        for (int j = 0; j < STATE_SIZE; j++) {
            P[i][j] = P_temp[i][j];
        }
    }
}

int main(void) {
    float z[MEASUREMENT_SIZE] = {0};
    
    predict_state();
    predict_covariance();
    update_state(z);
    
    LOG_DEBUG("Position: (%f, %f, %f)\n", x[0], x[1], x[2]);
    LOG_DEBUG("Velocity: (%f, %f, %f)\n", x[3], x[4], x[5]);
    LOG_DEBUG("Angles: (%f, %f, %f)\n", x[6], x[7], x[8]);
    
    return 0;
}