![Midnight Sun Banner](banner.png)
# Midnight Sun XVI Firmware Documentation

Welcome to the firmware documentation for the **Midnight Sun XVI** Solar Race Car, developed by the [University of Waterloo](https://uwaterloo.ca/)'s [Midnight Sun Solar Car Team](http://www.uwmidsun.com/).

## Projects Overview

The Midnight Sun XVI firmware is divided into several key projects:

- **Battery Management System (BMS)**: Manages the battery charging, discharging, state of charge (SoC) and overall battery health monitoring.
  
- **Power Distribution (PD)**: Responsible for managing power distribution to various subsystems of the vehicle.

- **Centre Console (CC)**: Manages the vehicle's driver control interface, providing communication between the driver and the car’s systems.

- **Motor Controller Interface (MCI)**: Provides communication and control for the motor controllers to manage the vehicle’s drive system.

- **Telemetry (TEL)**: Controls data over radio connection from the vehicle in real-time.

- **Flash-over-CAN (BOOTLOADER)**: A bootloader for updating the firmware of the car's embedded systems over the CAN bus.

- **UART Controller (UART CLI)**: A UART-based command-line interface that allows for MCU control.

- **IMU Board**: Interfaces with an IMU for testing. Provides critical data such as acceleration, orientation, and angular velocity.

- **CAN Autogen**: Automated generation of CAN messages, which simplifies the integration of various CAN devices in the system.

## Documentation Structure

This documentation provides a detailed overview of each firmware module, including architecture, functionality, and how to use it. The following sections are available for browsing:

- [Modules](modules.html): Descriptions of each module and its role within the firmware.
- [Classes](annotated.html): Descriptions of each struct and its role within the firmware.
- [Files](files.html): All files included in the firmware.
- [Setup and Installation](https://uwmidsun.atlassian.net/wiki/spaces/Firmware/pages/60260353/Setup): Instructions on how to set up the development environment.

## Getting Started

To get started with the firmware, please refer to the [installation guide](https://uwmidsun.atlassian.net/wiki/spaces/Firmware/pages/60260353/Setup) for instructions on setting up the development environment and flashing the firmware onto the vehicle's embedded systems.

## License

This project is licensed under the MIT License.
