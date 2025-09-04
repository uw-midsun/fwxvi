function ekf_out = ekf(sys, u, z, dt)
%EKF  Minimal Extended Kalman Filter for SOC estimation
%
%   ekf_out = ekf(sys, u, z, dt)
%
%   Inputs:
%       sys : struct from soc_state_space_model(params, dt)
%       u   : input current vector [Nx1] (A, +ve charging)
%       z   : measured terminal voltage [Nx1] (V)
%       dt  : sample period (s)
%
%   Output ekf_out:
%       .x_hat  [2xN] state estimates (SOC, Vrc)
%       .P      [2x2xN] covariance history
%
%   States:
%       x(1) = SOC (0–1)
%       x(2) = Vrc (V)

    N  = length(u);
    nx = 2;

    % --- Initial state from first few samples ---
    x0 = estimate_initial_state(sys.params, z(1:5), u(1:5));

    % --- Fixed covariances ---
    P0 = diag([0.01, 0.01]);
    Q  = diag([1e-7, 1e-5]);   % process noise
    R  = 1e-3;                 % measurement noise

    % --- Storage ---
    x_hat  = zeros(nx,N);
    P_hist = zeros(nx,nx,N);

    % init
    x_hat(:,1)    = x0;
    P_hist(:,:,1) = P0;

    rest_counter = 0;   % rest detection

    for k = 2:N
        % ===== Prediction =====
        x_pred = sys.state_update(x_hat(:,k-1), u(k-1));
        Fd     = sys.state_update_jacobian(x_hat(:,k-1), u(k-1));
        P_pred = Fd * P_hist(:,:,k-1) * Fd' + Q;

        % ===== Update =====
        H      = sys.output_jacobian(x_pred, u(k));
        y_pred = sys.output_function(x_pred, u(k));
        innov  = z(k) - y_pred;

        S      = H * P_pred * H' + R;
        K      = P_pred * H' / S;

        x_hat(:,k)    = x_pred + K * innov;
        P_hist(:,:,k) = (eye(nx) - K*H) * P_pred;

        % ===== Rest-based reset =====
        if abs(u(k)) < 0.05   % near-zero current
            rest_counter = rest_counter + 1;
        else
            rest_counter = 0;
        end

        if rest_counter >= 10   % e.g. 10 samples = 10 s if dt=1
            V_avg   = mean(z(k-9:k));
            soc_ocv = ocv_to_soc(sys.params, V_avg);

            x_hat(1,k) = soc_ocv;
            x_hat(2,k) = 0;          % reset Vrc toward 0
            P_hist(1,1,k) = 1e-6;    % tighten SOC variance

            rest_counter = 0;
        end
    end

    ekf_out.x_hat = x_hat;
    ekf_out.P     = P_hist;
end


%% --- Helper: initial state ---
function x0 = estimate_initial_state(params, V_samples, I_samples)
    V_mean = mean(V_samples);
    I_mean = mean(I_samples);

    V_ocv_est = V_mean + params.R0 * I_mean;
    soc0 = ocv_to_soc(params, V_ocv_est);

    Vrc0 = params.ocv_fun(soc0) - V_mean - params.R0 * I_mean;
    Vrc0 = min(max(Vrc0, -1), 1);  % clamp

    x0 = [soc0; Vrc0];
end

%% --- Helper: invert OCV -> SOC ---
function soc = ocv_to_soc(params, V_ocv_meas)
    soc = interp1(params.OCV_table, params.SOC_table, V_ocv_meas, ...
                  'linear','extrap');
    soc = min(max(soc,0),1);
end
