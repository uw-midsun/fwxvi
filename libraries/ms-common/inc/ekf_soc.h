#include <stdint.h>
#include "state_of_charge/h"
#define EKF_SOC_STATE_SIZE 1
#define MEASUREMENT_SIZE 4

//state matrix
double x[STATE_SIZE]={0};
//state covariance matrix
double P[STATE_SIZE][STATE_SIZE]={0};
//input controls (ax, ay, az)
double U[MEASUREMENT_SIZE]={0};

double B[MEASUREMENT_SIZE][MEASUREMENT_SIZE];

double A //4x4 diagonals are dt
//process noise and noise state covariance matrix
double Q[STATE_SIZE][STATE_SIZE]={0}; //9x9 matrix w/covariance in position, velocity, and orientation angle noise

void multiply_matrices(double firstMatrix[][STATE_SIZE], double secondMatrix[][STATE_SIZE], double result[][STATE_SIZE]);
void transponse_matrix(double matrix[][STATE_SIZE], double transpose[][STATE_SIZE]);


void noise_covariance();
void predict_state();
void predict_prev_state();//pk-1|k-1
void predict_covariance();//pk|k-1
void update_state();



double R[MEASUREMENT_SIZE][MEASUREMENT_SIZE]=
{{1, 0, 0, 0, 0, 0},
{0, 1, 0, 0, 0, 0},
{0, 0, 1, 0, 0, 0},
{0, 0, 0, 1, 0, 0},
{0, 0, 0, 0, 1, 0},
{0, 0, 0, 0, 0, 1}
};//since readings are independent from each other, just initialize to an identity matrix initially

typedef struct {
  float soc;      // State of Charge (0.0 - 1.0)
  float P;        // Error covariance
  float Q;        // Process noise
  float R;        
} EkfSoC;

void ekf_soc_init(EkfSoC *ekf, float initial_soc);
float ekf_soc_predict(EkfSoC *ekf, float current_a, float dt_s, float battery_capacity_as);
float ekf_soc_update(EkfSoC *ekf, float measured_soc);
