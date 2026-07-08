# fota

The bootloader for the telemetry board. It is the `can_bootloader` core with the XBee UART
wired in as the upstream transport, so it can relay a host firmware update from the XBee onto
the CAN bus (`laptop -> XBee -> fota -> CAN -> leaf nodes`).

It boots like `can_bootloader`: on a normal reset it jumps to the telemetry app once the boot
window expires. When the running app drops it back in (the host sends an enter command over the
XBee), it stays in the bootloader and bridges until a jump command is sent, then runs telemetry
again.

## Layout

- `src/main.c` wires the ms-bootloader core with CAN downstream and the XBee UART upstream.
- `src/bl_port_uart_xbee.c` is the real UART port for the XBee on USART2.
- `src/bl_port_stm32l4.c` is duplicated from `can_bootloader` for the MVP (CAN, flash, jump).
  It should be shared with `can_bootloader` rather than forked once this lands.
