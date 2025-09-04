function sys = soc_state_space_model(params, time_step)
    % State space model for state of charge
    %
    % =========================================================================
    % PHYSICAL INTERPRETATION
    % =========================================================================
    % 
    % This model represents a 1st-order equivalent circuit battery model:
    %
    %   V_ocv(SOC) ──[R0]──┬──[R1]──┬── V_terminal
    %                      │        │
    %                      └──[C1]──┴
    %
    % - R0: Ohmic internal resistance [Ohm]
    % - R1-C1: RC pair modeling diffusion/polarization effects
    %          (R1 in Ohm, C1 in Ah)
    % - V_ocv(SOC): Nonlinear open-circuit voltage vs state-of-charge [V]

    % =========================================================================
    % CONTINUOUS-TIME DYNAMICS:
    % =========================================================================
    % GENERAL STATE SPACE MODEL:
    %   x* = Ax + Bu
    %   y  = Cx + Du
    %
    % Continuous-time dynamics:
    %
    %   States (x):
    %       x1 = SOC   (state-of-charge, unitless 0.0 - 1.0)
    %       x2 = Vrc   (voltage across RC branch [V])
    %
    %   Input (u):
    %       u1 = Ipack (pack current [A], positive for charging)
    %
    %   Output (y):
    %       y1 = V_terminal = V_ocv(SOC) - R0*Ipack - Vrc
    %
    %   State equations:
    %       dSOC/dt = (1 / Q_pack_C) * Ipack
    %           where: Q_pack_C = Q_pack_Ah * 3600  % Convert Ah -> Coulombs
    %
    %       dVrc/dt = (1 / C1) * Ipack - (1 / (R1*C1)) * Vrc
    %
    %   Continuous-time matrices
    %       A = [  0              0
    %              0   -(1/(R1*C1)) ]
    %
    %       B = [ 1/Q_pack_C
    %              1/C1       ]
    %
    %       C = [ dV_ocv/dSOC   -1 ]
    %       D = [ -R0 ]

    % =========================================================================
    % DISCRETE-TIME DYNAMICS 
    % =========================================================================
    %
    %   State equations
    %       SOC[k+1] = SOC[k] + (Δt / Q_pack_C) * Ipack[k]
    %       Vrc[k+1] = Vrc[k] * (1 - Δt/(R1*C1)) + (Δt / C1) * Ipack[k]
    %
    %   Output equation:
    %       V_terminal[k] = V_ocv(SOC[k]) - R0*Ipack[k] - Vrc[k]

    assert(isfield(params,'Q_pack_Ah'),'params.Q_pack_Ah required');
    assert(isfield(params,'R0') && isfield(params,'R1') && isfield(params,'C1'),'R0,R1,C1 required');
    assert(isfield(params,'ocv_fun') && isa(params.ocv_fun,'function_handle'),'params.ocv_fun (function handle) required');

    Q_pack_Ah = params.Q_pack_Ah;
    Q_pack_C   = Q_pack_Ah * 3600;    % Ah -> Coulombs (A·s)
    R0 = params.R0;
    R1 = params.R1;
    C1 = params.C1;
    rc_pole = 1/(R1*C1);              % 1/s (pole of RC branch)

    ocv_fun = params.ocv_fun;
    ocv_grad = params.ocv_grad;

    % ---------- Continuous-time dynamics ----------
    % Note: x = [ SOC, Vrc ]
    %       u = [ Ipack]

    % --- State equations ---
    state_derivative = @(x,u) [ (1/Q_pack_C) * u;              % dSOC/dt
                                (1/C1) * u - rc_pole * x(2) ]; % dVrc/dt

    state_jacobian   = @(x,u) [ 0, 0;
                                0, -rc_pole ];

    % --- Measurement function (terminal voltage) ---
    output_function  = @(x,u) ocv_fun(clamp01(x(1))) - R0*u - x(2);
    output_jacobian  = @(x,u) [ ocv_grad(clamp01(x(1))), -1 ];

    % ---------- Discrete-time dynamics ----------
    exp_term = exp(-rc_pole * dt);
    K_vrc    = R1 * (1 - exp_term);

    state_update     = @(x,u) [ x(1) + dt*(1/Q_pack_C)*u;   % SOC[k+1]
                                exp_term*x(2) + K_vrc*u ];  % Vrc[k+1]

    state_update_jac = @(x,u) [ 1, 0;
                                0, exp_term ];

    % --- package ---
    sys = struct();
    sys.state_derivative      = state_derivative;     % f(x,u)
    sys.state_jacobian        = state_jacobian;       % F(x,u)
    sys.output_function       = output_function;      % h(x,u)
    sys.output_jacobian       = output_jacobian;      % H(x,u)
    sys.state_update          = state_update;         % fd(x,u)
    sys.state_update_jacobian = state_update_jac;     % Fd(x,u)
    sys.params                = params;
end

function z = clamp01(x)
    z = min(max(x,0),1);
end
