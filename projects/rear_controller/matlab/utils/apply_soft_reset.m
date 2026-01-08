% =========================================================================
% @file    apply_soft_reset.m
% @brief   Soft reset logic for EKF state and covariance
% @date    2025-09-07
% @author  Midnight Sun Team #24 - MSXVI
%
% =========================================================================
% FUNCTION DESCRIPTION
% =========================================================================
% Blends SOC toward the OCV-derived value and Vrc toward zero, while also
% tightening the covariance matrix. Useful for improving filter stability
% during rest phases
%
% -------------------------------------------------------------------------
% INPUTS:
%   x_in    : double [2x1]
%       Prior state vector [SOC; Vrc]
%   P_in    : double [2x2]
%       Prior covariance matrix
%   V_avg   : double [scalar]
%       Average terminal voltage during rest
%   params  : struct
%       Battery parameters (requires OCV lookup tables)
%   alpha   : double [scalar, 0-1]
%       Reset blending factor (0 = keep prior, 1 = full reset)
%
% OUTPUTS:
%   x_out   : double [2x1]
%       Blended/reset state
%   P_out   : double [2x2]
%       Tightened covariance
%
% =========================================================================

function [x_out, P_out] = apply_soft_reset(x_in, P_in, V_avg, params, alpha) %#codegen
    soc_ocv = ocv_to_soc(params, V_avg);

    x_out = x_in;
    x_out(1) = (1.0 - alpha) * x_in(1) + alpha * soc_ocv;
    x_out(2) = (1.0 - alpha) * x_in(2);

    % shrink covariance (tune factor as needed)
    P_out = P_in * 0.5;
end
