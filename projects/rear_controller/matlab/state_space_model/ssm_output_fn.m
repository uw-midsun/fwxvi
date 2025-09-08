% =========================================================================
% @file    ssm_output_fn.m
% @brief   Battery state-space model output function
% @date    2025-09-07
% @author  Midnight Sun Team #24 - MSXVI
%
% =========================================================================
% FUNCTION DESCRIPTION
% =========================================================================
% Computes the terminal voltage of the 2-state RC battery model:
%
%   y = OCV(SOC) - Vrc - R0 * I
%
% -------------------------------------------------------------------------
% INPUTS:
%   x       : double [2x1]
%       Current state vector [SOC; Vrc]
%   u       : double [scalar]
%       Current input (battery current, Amperes)
%   params  : struct
%       Battery model parameters (must include R0)
%
% OUTPUT:
%   y       : double [scalar]
%       Terminal voltage [V]
%
% =========================================================================

function y = ssm_output_fn(x, u, params)
    soc = x(1);
    Vrc = x(2);

    ocv_pack = ocv_from_soc(params, soc);

    R0 = params.R0_pack;
    y = ocv_pack - Vrc - R0 * u;
end
