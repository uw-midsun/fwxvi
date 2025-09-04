% test/simulate_soc.m
function simulate_soc()
    % --- Setup ---
    addpath('..');   % so it can see battery_params.m, soc_state_space_model.m, ekf.m

    params = battery_params();
    dt = 1.0; % sample time [s]
    N  = 1000; % number of steps
    sys = soc_state_space_model(params, dt);

    % --- Generate a synthetic current profile (A) ---
    % discharge at 1C for 200s, rest 100s, charge at 0.5C for 200s, etc.
    Ipack = zeros(N,1);
    Ipack(1:200)   = -params.Q_pack_Ah * 1.0;   % discharge 1C
    Ipack(301:500) =  params.Q_pack_Ah * 0.5;   % charge 0.5C
    Ipack(701:900) = -params.Q_pack_Ah * 0.2;   % discharge 0.2C

    % --- Simulate true system (plant) ---
    x_true = zeros(2,N);
    z_true = zeros(N,1);

    x_true(:,1) = [1.0; 0];  % start full SOC, zero Vrc
    for k = 2:N
        x_true(:,k) = sys.state_update(x_true(:,k-1), Ipack(k-1));
        z_true(k)   = sys.output_function(x_true(:,k), Ipack(k));
    end

    % --- Add measurement noise ---
    noise_std = 0.01; % 10 mV noise
    z_meas = z_true + noise_std * randn(N,1);

    % --- Run EKF ---
    ekf_out = ekf(sys, Ipack, z_meas, dt);

    % --- Benchmark = true SOC from simulation ---
    soc_true = x_true(1,:);
    soc_est  = ekf_out.x_hat(1,:);

    % --- Plot results ---
    figure;
    subplot(2,1,1);
    plot(soc_true,'k','LineWidth',1.5); hold on;
    plot(soc_est,'r--','LineWidth',1.2);
    legend('True SOC','EKF SOC');
    ylabel('SOC (0–1)');
    grid on;

    subplot(2,1,2);
    plot(z_meas,'b'); hold on;
    plot(z_true,'k','LineWidth',1.5);
    legend('Measured V','True V');
    ylabel('Terminal Voltage (V)');
    xlabel('Sample');
    grid on;
end
