# Extended Kalman Filter for Battery SOC Estimation

The EKF is like a smart way to combine two sources of information:
1. **Our battery model prediction** - "Based on physics, here's where I think the SOC should be"
2. **Voltage measurement** - "Here's what I actually measured from the battery"

The EKF automatically decides how much to trust each source and blends them optimally.

## Why Do We Need It?

**The Problem:**
- We can't directly measure SOC - it's inside the battery
- We can only measure terminal voltage, which depends on SOC + other effects
- Our battery model isn't perfect
- Our measurements have noise

**The Solution:**
The EKF continuously estimates the hidden states (SOC and Vrc) by:
- Using the battery model to predict what should happen
- Comparing predictions with actual voltage measurements
- Adjusting estimates when predictions don't match measurements

## How It Works

### The EKF Cycle (Every Time Step)

#### 1. PREDICT Step
```
"Where do I think the battery will be next?"
```
- Use the battery model to predict next SOC and Vrc
- Account for uncertainty in the model (process noise)
- Result: Predicted states + how uncertain we are

#### 2. UPDATE Step  
```
"How should I adjust my prediction based on what I measured?"
```
- Measure actual terminal voltage
- Compare with what we predicted the voltage should be
- Calculate the "innovation" (how wrong we were)
- Use Kalman gain to blend prediction with measurement
- Result: Updated states + reduced uncertainty

### Key Matrices

**State Vector (x):**
```
x = [SOC]    - What we're trying to estimate
    [Vrc]
```

**Process Noise (Q):**
```
Q = [1e-6   0  ]  - How much we trust our battery model
    [0    1e-5]   - Smaller = more trust
```

**Measurement Noise (R):**
```
R = 1e-3  - How much we trust our voltage sensor (~32mV std dev)
```

**Kalman Gain (K):**
- Automatic weight between prediction and measurement
- High K = trust measurement more
- Low K = trust prediction more

## Integration with Battery SSM

The EKF uses several functions from the battery SSM:

### From SSM to EKF:
- `ssm_state_update()` → Predict next states
- `ssm_state_jacobian()` → Linearize state transition  
- `ssm_output_fn()` → Predict voltage measurement
- `ssm_output_jacobian()` → Linearize measurement equation

### Flow:
```
Previous State → SSM Prediction → EKF Update → New State
     ↑                                            ↓
     └────────────────────────────────────────────┘
```

## Tuning Parameters

### Process Noise (Q)
- **Q(1,1)**: SOC model uncertainty
  - Smaller → trust battery capacity model more
  - Larger → allow more SOC adjustment from measurements
- **Q(2,2)**: RC voltage model uncertainty  
  - Smaller → trust RC dynamics more
  - Larger → allow more Vrc adjustment

### Measurement Noise (R)
- **R**: Voltage sensor uncertainty
  - Smaller → trust voltage measurements more
  - Larger → rely more on model predictions
  - Typical: 1e-3 (32mV std dev) to 1e-4 (10mV std dev)

## Typical Performance

**Good Conditions** (accurate model, clean measurements):
- SOC error: < 2%
- Convergence time: 10-30 seconds

**Challenging Conditions** (model mismatch, noisy measurements):
- SOC error: 3-5% 
- Convergence time: 1-2 minutes

## Usage Example

```matlab
% Initialize
x = [0.8; 0.0];           % Start at 80% SOC, no RC voltage
P = eye(2) * 0.01;        % Initial uncertainty
params = battery_params(); % Load battery parameters

% Each time step
for k = 1:N
    [x, P, y_pred] = ekf_step(x, P, current(k), voltage(k), params, dt);
    soc_estimate(k) = x(1);
end
```

## Common Issues

**Divergence**: EKF estimates drift away from truth
- **Cause**: Q too small (over-confident in model)
- **Fix**: Increase process noise

**Slow Convergence**: Takes too long to reach correct SOC
- **Cause**: R too large (don't trust measurements)  
- **Fix**: Reduce measurement noise (if sensor is actually good)

**Noisy Estimates**: SOC jumps around too much
- **Cause**: R too small (trust noisy measurements too much)
- **Fix**: Increase measurement noise to match actual sensor performance