% =========================================================================
% @file    clamp01.m
% @brief   Clamp scalar value to [0, 1]
% @date    2025-09-07
% @author  Midnight Sun Team #24 - MSXVI
%
% =========================================================================
% FUNCTION DESCRIPTION
% =========================================================================
% Restricts input to the range [0, 1]. Commonly used for SOC bounds
%
% -------------------------------------------------------------------------
% INPUT:
%   x   : double [scalar]
%       Input value
%
% OUTPUT:
%   v   : double [scalar]
%       Clamped value in [0, 1]
%
% =========================================================================

function v = clamp01(x) %#codegen
    v = min(max(x, 0.0), 1.0);
end
