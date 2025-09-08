% =========================================================================
% @file    ssm_state_jacobian.m
% @brief   State Jacobian for battery state-space model
% @date    2025-09-07
% @author  Midnight Sun Team #24 - MSXVI
%
% =========================================================================
% FUNCTION DESCRIPTION
% =========================================================================
% Computes Jacobian of the discrete-time state update function
% For this RC battery model, the matrix is diagonal:
%
%   F = [ 1,    0 ;
%         0, alpha ]
%
% where alpha = exp(-dt / (R1*C1))
%
% -------------------------------------------------------------------------
% INPUTS:
%   x       : double [ignored]
%   u       : double [ignored]
%   params  : struct
%       Battery model parameters (R1, C1 required)
%   dt      : double [scalar]
%       Sampling time step [s]
%
% OUTPUT:
%   F       : double [2x2]
%       State transition Jacobian
%
% =========================================================================

function F = ssm_state_jacobian(~, ~, params, dt)
    R1 = params.R1_pack;
    C1 = params.C1_pack;
    tau = R1 * C1;

    if tau <= 0
        alpha = 0.0;
    else
        alpha = exp(-dt / tau);
    end

    F = [1.0, 0.0;
         0.0, alpha];
end
