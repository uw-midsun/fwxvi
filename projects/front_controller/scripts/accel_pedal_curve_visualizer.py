# @file    accel_pedal_curve_visualizer.py
#  @date    2025-03-12
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Visualizes the acceleration pedal curve
#
#  @ingroup Front_Controller

# To run this do: python3 projects/front_controller/scripts/accel_pedal_curve_visualizer.py --csv_file=[INSERT PATH TO CSV FILE]

import argparse
import os
import pandas as pd
import matplotlib.pyplot as plt


def check_csv_format(data_frame):
    """
    @brief Check the CSV has the expected columns
    """
    # List of expected columns
    expected_columns = ['adc_input', 'normalized', 'after_deadzone', 'after_curve', 'remapped_output']

    for col in expected_columns:
        if col not in data_frame.columns:
            raise ValueError(f"Missing expected column: {col}")

    print("CSV format is valid.")


def visualize_csv(csv_file):
    """
    @brief Visualize the pedal data in the CSV file
    """
    data_frame = pd.read_csv(csv_file)

    check_csv_format(data_frame)

    # Plot for adc_input vs normalized
    plt.figure(figsize=(10, 6))
    plt.subplot(2, 2, 1)
    plt.plot(data_frame['adc_input'], data_frame['normalized'], label='Normalized', color='b')
    plt.xlabel('ADC Input')
    plt.ylabel('Normalized')
    plt.title('ADC Input vs Normalized')
    plt.grid(True)

    # Plot for adc_input vs after_deadzone
    plt.subplot(2, 2, 2)
    plt.plot(data_frame['adc_input'], data_frame['after_deadzone'], label='After Deadzone', color='g')
    plt.xlabel('ADC Input')
    plt.ylabel('After Deadzone')
    plt.title('ADC Input vs After Deadzone')
    plt.grid(True)

    # Plot for adc_input vs after_curve
    plt.subplot(2, 2, 3)
    plt.plot(data_frame['adc_input'], data_frame['after_curve'], label='After Curve', color='r')
    plt.xlabel('ADC Input')
    plt.ylabel('After Curve')
    plt.title('ADC Input vs After Curve')
    plt.grid(True)

    # Plot for adc_input vs remapped_output
    plt.subplot(2, 2, 4)
    plt.plot(data_frame['adc_input'], data_frame['remapped_output'], label='Remapped Output', color='purple')
    plt.xlabel('ADC Input')
    plt.ylabel('Remapped Output')
    plt.title('ADC Input vs Remapped Output')
    plt.grid(True)

    plt.tight_layout()

    plt.show()


def main():
    """
    @brief Main function for acceleration pedal curve visualizer
    """
    print("Welcome to accel_pedal_curve_visualizer")

    parser = argparse.ArgumentParser(description="Visualize Accel Pedal Curve Data")
    parser.add_argument("--csv_file", help="Path to the CSV file containing the curve data")

    args = parser.parse_args()

    if not os.path.isfile(args.csv_file):
        print(f"Error: The file {args.csv_file} does not exist.")
        return
    try:
        visualize_csv(args.csv_file)
    except ValueError as e:
        print(f"Error: {e}")


if __name__ == "__main__":
    main()
