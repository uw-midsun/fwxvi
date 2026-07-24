# rc_current_sense

Bench smoke test for the rear controller pack current-sense chain built around the
**ADS122C14** ADC.

## What it does
1. Drives the three pack relay enables closed in sequence — **POS**, **SOLAR**, **NEG** —
   so the shunt and HV divider see live pack potential.
2. Initializes the ADS122 with the same register map / config used by
   `projects/rear_controller/src/current_sense.c`.
3. Continuously alternates the ADC MUX between the two channels and logs:
   - **PACK_CURRENT** — shunt path (AIN6/AIN7), `V_shunt / 0.5 mΩ`
   - **PACK_VOLTAGE** — HV divider path (AIN0/AIN1), scaled by `(R6 + R7) / R7`

It talks to the ADS122 driver and relay GPIOs directly (no state manager / CAN stack)
so a single board can be verified in isolation.

## Usage
```sh
# Build + flash on the rear controller MCU
scons --smoke=rc_current_sense --platform=arm
scons flash --smoke=rc_current_sense

# Compile / simulate on host (x86). Note: the x86 ADS122 stub never asserts the
# data-ready bit, so reads report a timeout — real hardware is required for values.
scons sim --smoke=rc_current_sense
```

## Wiring notes
- Current sense I2C: `I2C_PORT_3`, SDA `PC1`, SCL `PC0`, ADS122 addr `0x40`.
- Relay enables: POS `PA6`, SOLAR `PA8`, NEG `PA4` (active-high enable).
- Shunt is `0.5 mΩ` (0.5 mV/A); FSR is `±5 V` (Vref 2.5 V, gain 0.5).
