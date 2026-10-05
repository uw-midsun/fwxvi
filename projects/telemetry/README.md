# Telemetry

Telemetry records received Classic CAN data frames to the SD card in candump
format. Each boot creates `logs_<reboot_number>.log` on the mounted drive:

```text
(1.234000) can0 551#0102030405
(1.235000) can0 00000123#1122334455667788
```

The timestamp is seconds since boot, captured when the telemetry CAN callback
processes the frame. It has 1 ms resolution with the current RTOS configuration;
the six decimal places do not imply microsecond precision. Frames processed in
the same tick can have equal timestamps. Tick wraparound is accounted for.
Standard IDs use three hex digits and extended IDs use eight, including leading
zeros. The payload contains exactly the received DLC's number of bytes.

The callback copies each frame into a separate 128-entry queue and continues
normal CAN/WS22 processing. It never waits for the SD card. The SD task batches
frames for up to 20 ms, writes through a 1 KiB buffer, and keeps the file open.
It calls `f_sync()` approximately once per second, including when traffic stops.
Sudden power loss can lose recent unsynced data. An SD open, write, or sync error
stops logging until reboot and reports the failure over the debug output.

If the logging queue fills, the newest frame is dropped. Debug output reports
the cumulative count, also available through `telemetry_log_dropped()`. This
counts only drops in the SD logging queue, not upstream CAN hardware/RX queue
losses. The capture includes frames delivered to the telemetry receive callback;
it does not independently capture this board's transmitted frames, CAN error
events, or remote-frame metadata. The current CAN message interface does not
preserve the RTR flag. SD writes still use the standalone HAL SPI driver, with
the IMU disabled.

## Replay and decode

On Linux with `can-utils` installed, create a virtual CAN interface once:

```sh
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set vcan0 up
```

Watch traffic in one terminal and replay the recording in another:

```sh
candump vcan0
canplayer -I logs_3.log vcan0=can0
```

The assignment maps the recorded `can0` channel to the laptop's `vcan0`. Replay
uses recorded timestamp differences. A DBC is not needed to replay raw frames.
To decode the recording into signals, use a matching DBC with `cantools`:

```sh
python3 -m cantools decode can/tools/system_dbc.dbc < logs_3.log
```

The DBC must match the firmware/messages used during capture; it may not cover
every received ID. The old once-per-second signal CSV is no longer produced.

## Build and verify

```sh
scons --platform=arm --project=telemetry
scons test --platform=x86 --project=telemetry
```

Host logger tests mock FatFs and the callback clock while using the RTOS queue.
They check exact log records, timestamps, overflow, batched writes, and SD errors.
Hardware validation still requires a formatted SD card and incoming CAN traffic.
