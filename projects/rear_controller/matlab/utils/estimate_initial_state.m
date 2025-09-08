% =========================================================================
% @file    estimate_initial_state.m
% @brief   Estimate initial battery state (SOC, Vrc)
% @date    2025-09-07
% @author  Midnight Sun Team #24 - MSXVI
%
% =========================================================================
% FUNCTION DESCRIPTION
% =========================================================================
% Uses the average of the first few voltage samples to estimate SOC, assumes
% the cell is initially at rest (Vrc = 0). Provides a reasonable initial
% guess for filter startup
%
% -------------------------------------------------------------------------
% INPUTS:
%   params     : struct
%       Battery parameters (must include OCV/SOC tables)
%   v_samples  : double [vector]
%       Voltage samples from startup window
%   u_samples  : double [vector]
%       Current samples (not used here, but available for validation)
%
% OUTPUT:
%   x0         : double [2x1]
%       Initial state vector [SOC0; Vrc0]
%
% =========================================================================

function x0 = estimate_initial_state(params, v_samples, u_samples)
    V_avg = mean(v_samples(:));
    soc0 = ocv_to_soc(params, V_avg);

    x0 = zeros(2,1,'like',V_avg);
    x0(1) = clamp01(soc0);
    x0(2) = single(0.0);
end
