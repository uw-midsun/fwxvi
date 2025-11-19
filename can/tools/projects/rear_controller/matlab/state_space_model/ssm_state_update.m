% =========================================================================
% @file    ssm_state_update.m
% @brief   State update function for 2-state RC battery model
% @date    2025-09-07
% @author  Midnight Sun Team #24 - MSXVI
%
% =========================================================================
% FUNCTION DESCRIPTION
% =========================================================================
% Discrete-time update of SOC and RC voltage states:
%
%   SOC_next = clamp01(SOC + (I*dt) / (Q_Ah * 3600))
%   Vrc_next = alpha * Vrc + R1 * (1 - alpha) * I
%
% where alpha = exp(-dt / (R1*C1)).
%
% -------------------------------------------------------------------------
% INPUTS:
%   x       : double [2x1]
%       Current state vector [SOC; Vrc]
%   u       : double [scalar]
%       Current input (battery current, Amperes)
%   params  : struct
%       Battery model parameters (must include R1, C1, Q_pack_Ah or Q_Ah)
%   dt      : double [scalar]
%       Sampling time step [s]
%
% OUTPUT:
%   x_next  : double [2x1]
%       Next state vector [SOC_next; Vrc_next]
%
% =========================================================================

function x_next = ssm_state_update(x, u, params, dt) %#codegen
    SOC = x(1);
    Vrc = x(2);

    Q_Ah = params.Q_pack_Ah;

    R1 = params.R1_pack;
    C1 = params.C1_pack;

    % SOC integration (Coulomb counting, pack-level)
    dSOC = (u * dt) / (Q_Ah * 3600.0);
    SOC_next = clamp01(SOC + dSOC);

    % RC branch dynamics
    tau = R1 * C1;
    if tau <= 0
        alpha = 0.0;
    else
        alpha = exp(-dt / tau);
    end
    K_vrc = R1 * (1.0 - alpha);
    Vrc_next = alpha * Vrc + K_vrc * u;

    x_next = [SOC_next; Vrc_next];
end
