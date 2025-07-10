#pragma once
#include <stdio.h>
#include <math.h>
#define STATE_SIZE 9
#define MEASUREMENT_SIZE 6 //(x, y, z), (ax, ay, az), (a, b, y)
#define DT 1

#define MEASUREMENT_SIZE_SOC 4




//state matrix
double x[STATE_SIZE]={0};
//state covariance matrix
double P[STATE_SIZE][STATE_SIZE]={0};
//input controls (ax, ay, az)
double U[MEASUREMENT_SIZE]={0};


//process noise and noise state covariance matrix
double Q[STATE_SIZE][STATE_SIZE]={0}; //9x9 matrix w/covariance in position, velocity, and orientation angle noise


double R[MEASUREMENT_SIZE][MEASUREMENT_SIZE]=
{{1, 0, 0, 0, 0, 0},
{0, 1, 0, 0, 0, 0},
{0, 0, 1, 0, 0, 0},
{0, 0, 0, 1, 0, 0},
{0, 0, 0, 0, 1, 0},
{0, 0, 0, 0, 0, 1}
};//since readings are independent from each other, just initialize to an identity matrix initially


extern double A[STATE_SIZE][STATE_SIZE] = {
    {1, 0, 0, DT, 0, 0,  0,  0,  0},
    {0, 1, 0, 0, DT, 0,  0,  0,  0},
    {0, 0, 1, 0, 0, DT,  0,  0,  0},
    {0, 0, 0, 1, 0, 0,   0,  0,  0},
    {0, 0, 0, 0, 1, 0,   0,  0,  0},
    {0, 0, 0, 0, 0, 1,   0,  0,  0},
    {0, 0, 0, 0, 0, 0,   1, DT, 0},
    {0, 0, 0, 0, 0, 0,   0, 1, DT},
    {0, 0, 0, 0, 0, 0,   0, 0, 1}
};//iniitialzie with time derivatives
double B[MEASUREMENT_SIZE][MEASUREMENT_SIZE];




void multiply_matrices(double firstMatrix[][STATE_SIZE], double secondMatrix[][STATE_SIZE], double result[][STATE_SIZE]);
void transponse_matrix(double matrix[][STATE_SIZE], double transpose[][STATE_SIZE]);


void noise_covariance();
void predict_state();
void predict_prev_state();//pk-1|k-1
void predict_covariance();//pk|k-1
void update_state();
