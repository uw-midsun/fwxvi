# Battery State Space Model

This document explains the battery state space model used for SOC (State of Charge) estimation.

## Physical Model

The battery is modeled as a simple equivalent circuit:

```
V_ocv(SOC) ──[R0]──┬──[R1]──┬── V_terminal
                   │        │
                   └──[C1]──┴
```

**Components:**
- **R0**: Ohmic internal resistance [Ohm] - immediate voltage drop
- **R1-C1**: RC pair modeling diffusion/polarization effects
  - R1 in Ohm, C1 in Ah (ampere-hours)
- **V_ocv(SOC)**: Open-circuit voltage as a function of SOC [V]

## States and Inputs

**States (what we track):**
- `SOC`: State of charge (0.0 = empty, 1.0 = full)
- `Vrc`: Voltage across the RC branch [V]

**Input:**
- `Ipack`: Battery current [A] (positive = charging, negative = discharging)

**Output:**
- `V_terminal`: Terminal voltage [V] that we can measure

## How It Works

### Terminal Voltage Calculation
The terminal voltage is calculated as:
```
V_terminal = V_ocv(SOC) - R0*Ipack - Vrc
```

This means:
- Start with the open-circuit voltage for the current SOC
- Subtract the immediate ohmic drop (R0 × current)
- Subtract the RC branch voltage

### State Updates

**SOC Update:**
```
SOC[k+1] = SOC[k] + (Δt / Q_pack_C) * Ipack[k]
```
- SOC changes based on how much current flows
- `Q_pack_C` is the battery capacity in Coulombs (Ah × 3600)

**RC Voltage Update:**
```
Vrc[k+1] = Vrc[k] * exp(-Δt/(R1*C1)) + K_vrc * Ipack[k]
```
- The RC branch has exponential decay with time constant `R1*C1`
- Current adds to the RC voltage with gain `K_vrc = R1 * (1 - exp(-Δt/(R1*C1)))`

## Continuous vs Discrete Time

### Continuous Time (for analysis)
State equations:
```
dSOC/dt = (1 / Q_pack_C) * Ipack
dVrc/dt = (1 / C1) * Ipack - (1 / (R1*C1)) * Vrc
```

### Discrete Time (for simulation)
State updates:
```
SOC[k+1] = SOC[k] + (Δt / Q_pack_C) * Ipack[k]
Vrc[k+1] = Vrc[k] * (1 - Δt/(R1*C1)) + (Δt / C1) * Ipack[k]
```

## Required Parameters

The model needs these parameters in the `params` struct:

- `Q_pack_Ah`: Battery capacity in ampere-hours
- `R0`: Ohmic resistance [Ohm]
- `R1`: RC branch resistance [Ohm]  
- `C1`: RC branch capacitance [Ah]

## Key Functions Provided

- `state_derivative`: Continuous-time dynamics f(x,u)
- `state_jacobian`: Jacobian matrix F(x,u) 
- `output_function`: Measurement equation h(x,u)
- `output_jacobian`: Measurement Jacobian H(x,u)
- `state_update`: Discrete-time state update fd(x,u)
- `state_update_jacobian`: Discrete-time Jacobian Fd(x,u)

## SOC Clamping

The SOC is automatically clamped between 0 and 1 using the `clamp01()` function to prevent unphysical values.

## Usage

This model is used by:
- Battery simulators for generating synthetic data
- EKF estimators for SOC estimation
- Model validation and parameter identification routines