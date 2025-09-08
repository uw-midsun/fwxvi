% =========================================================================
% @file    ssm_output_jacobian.m
% @brief   Output Jacobian for battery state-space model
% @date    2025-09-07
% @author  Midnight Sun Team #24 - MSXVI
%
% =========================================================================
% FUNCTION DESCRIPTION
% =========================================================================
% Computes Jacobian of the output equation with respect to state vector x:
%
%   H = [ dOCV/dSOC, -1 ]
%
% dOCV/dSOC is estimated by finite differencing on the OCV lookup table
%
% -------------------------------------------------------------------------
% INPUTS:
%   x       : double [2x1]
%       Current state vector [SOC; Vrc]
%   u       : double [scalar] (unused)
%       Current input (for signature consistency)
%   params  : struct
%       Battery model parameters
%
% OUTPUT:
%   H       : double [1x2]
%       Output Jacobian [dOCV/dSOC, -1]
%
% =========================================================================

function H = ssm_output_jacobian(x, ~, params)
    soc = x(1);
    eps_soc = 1e-5;

    soc_p = clamp01(soc + eps_soc);
    soc_m = clamp01(soc - eps_soc);

    v_p = ocv_from_soc(params, soc_p);
    v_m = ocv_from_soc(params, soc_m);

    dOCVdSOC = (v_p - v_m) / (soc_p - soc_m + 1e-12);
    H = [dOCVdSOC, -1.0];
end
