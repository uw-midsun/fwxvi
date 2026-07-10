# 01 — Existing Implementation

Snapshot of how the bootloader/bootstrap work on the `bootloader-debug` branch as
of 2026-05-30. This is descriptive, not prescriptive — see doc 02 for the
problems and doc 03 for the redesign.

---

## 1. Boot chain

Three stages live in flash, lowest address first:

```
reset ─▶ Bootstrap ─▶ Bootloader ─▶ Application
        (32 KB)      (64 KB)        (rest)
```

1. **Bootstrap** (`projects/bootstrap/`) — runs at the reset vector
   (`0x08000000`). Pulls a CRC value out of the on-flash filesystem, CRCs the
   bootloader region, validates the bootloader's vector table, and jumps to it.
   Falls back to jumping straight to the app (or halting) if anything fails.
2. **Bootloader** (`projects/bootloader/`) — bare-metal `while(1)` that polls CAN,
   feeds frames into a state machine, and performs DFU into the application region.
   After an inactivity timeout it jumps to the application.
3. **Application** — any normal project, built against the `application` linker preset.

### Jump mechanics

- **Bootstrap → bootloader** (`bootstrap.c:jump_to`): disables IRQs, sets
  `SCB->VTOR`, `DSB`/`ISB`, sets `MSP` from the target's `vector[0]`, branches to
  `vector[1]`. This is the *correct* sequence.
- **Bootloader → app** (`bootloader_dfu.c:bootloader_jump_app`): inline asm that
  loads `dfu_info.application_start`, reads the reset vector, and `BX`es to it.
  It does **not** set `MSP` or `VTOR` (see doc 02).

---

## 2. Components

### Bootstrap

| File | Role |
|------|------|
| `src/bootstrap.c` | CRC32 (software, reflected/zlib), vector-table validation, `jump_to`, `bootstrap_main`. |
| `inc/bootstrap_memory_map.h` | Region addresses/sizes; linker symbols on ARM, hardcoded constants on x86. |

### Bootloader

| File | Role |
|------|------|
| `src/main.c` | `SysTick` handler, CAN init, init, optional loopback self-test, the poll loop + inactivity timeout. |
| `src/bootloader.c` | State machine: arbitration-ID → state table, transition guards, per-state dispatch. |
| `src/bootloader_packet_handler.c` | `PacketManager` state; start / sequencing / data-receive / ping handlers; page-buffer assembly. |
| `src/bootloader_dfu.c` | Per-page flash (CRC-before, erase, write, read-back, CRC-after); jump-to-app; fault reset. |
| `src/bootloader_can.c` | STM32 HAL CAN init/tx/rx, bit-timing tables, bus-status checks. |
| `src/bootloader_can_datagram.c` | (De)serialize CAN frames ↔ `BootloaderDatagram`; send ACK/NACK. |
| `src/bootloader_crc32.c` | STM32 **hardware** CRC peripheral wrapper. |
| `src/bootloader_flash.c` | Double-word flash write, page erase, read, whole-app "is it blank?" check. |
| `src/bootloader_mpu.c` | MPU region config to lock the bootloader (currently never called). |

---

## 3. Datagram protocol (CAN)

Reserved arbitration IDs (`bootloader_can_datagram.h`):

| ID | Name | Direction | Frame layout (little-endian) |
|----|------|-----------|------------------------------|
| 30 | `SEQUENCING` | client→node | `[seq:u16][crc32:u32][pad:u16]` |
| 31 | `FLASH` | client→node | `[data:8 bytes]` |
| 32 | `JUMP_APPLICATION` | client→node | `[node_ids:u16]` |
| 33 | `ACK` | node→client | `[ack:u8][err_hi:u8][err_lo:u8]` |
| 34 | `START` | client→node | `[node_ids:u16][data_len:u32][pad:u16]` |
| 35 | `JUMP_BOOTLOADER` | client→node | (unused in firmware) |
| 36 | `PING_METADATA` | client→node | (partial) |
| 37 | `PING_DATA` | client→node | `[node_ids:u16][req:4|len:12][crc32:u32]` |

- `node_ids` is a 16-bit bitmask of target boards. The firmware deserializes it
  into `PacketManager.target_nodes`, **but the per-node filtering is commented out**,
  so every node currently acts on every command.
- All multi-byte fields are little-endian (STM32 native).

---

## 4. State machine

States (`bootloader.h:BootloaderStates`): `UNINITIALIZED, IDLE, START,
WAIT_SEQUENCING, DATA_RECEIVE, JUMP_APP, FAULT, PING`.

`bootloader_run(msg)` does, in order:
1. `deserialize_datagram(msg)` → fills the `datagram` union + `target_nodes`.
2. `s_switch_states(msg->id)` → maps the arbitration ID to a target state via
   `arbitration_id_handle[]` and applies a per-state transition guard.
3. `s_run_state()` → executes the handler for the current state.

Any guard violation or handler error drives the machine to `FAULT`, which sends a
NACK and calls `NVIC_SystemReset()`.

---

## 5. DFU flow (happy path)

```
client                          node (bootloader)
  │  START(len) ───────────────▶ validate 4-byte aligned, reset counters
  │  ◀─────────────── ACK
  │  SEQUENCING(seq=0, crc) ───▶ store chunk CRC, arm first-byte
  │  FLASH(8B) × N ────────────▶ append to 2 KB page buffer
  │                              when page full or last byte:
  │                                CRC-check buffer == chunk crc
  │                                erase 1 page, write, read back, CRC again
  │  ◀─────────────── ACK         (per page)
  │  SEQUENCING(seq=1, crc) ───▶ next 2 KB chunk …
  │            …                  repeat until bytes_written == len
  │  JUMP_APPLICATION ─────────▶ ACK, jump
```

- **Page size** is hardcoded to 2 KB (`BOOTLOADER_PAGE_BYTES = 0x800`) and
  `NUM_FLASH_PAGES = 128` (256 KB / 2 KB).
- Flash writes are 64-bit **double-word** programming (`FLASH_TYPEPROGRAM_DOUBLEWORD`).
- CRC validation happens **before and after** each page write.

---

## 6. Configuration / metadata storage

- The bootstrap reads `crc.txt` from the on-flash filesystem (`midFS`,
  `FS_HAL_ADDRESS = 0x0803C000`) and compares it to a freshly computed CRC of the
  bootloader region.
- There is **no** config region for board ID / name / project / app CRC — the
  metadata described in the original design doc is not implemented.
- `midFS` is used *only* by the bootstrap; no application uses it.

---

## 7. Build & flash plumbing

- **Presets** (`build_presets.json`): per chip × `{bootstrap, bootloader,
  application, legacy} × {debug, release}`. A preset selects a `flash` type.
- **Linker selection** (`platform/arm.py:get_link_flags`): `flash` type →
  `memory_map.ld` (`default` or `legacy`) + a `sections_<type>.ld` template that
  aliases `_flash_target_region_` to the right MEMORY region.
- **Memory maps** (`platform/hardware/<chip>/{default,legacy}/memory_map.ld`):
  define the MEMORY regions and export `_*_start` / `_*_size` linker symbols.
- **OpenOCD** (`platform/hardware/templates/stm32_flash_procs.tcl` +
  `<chip>/.../device_params.cfg`): per-region flash procs compute a start address
  from the region sizes and `flash write_image erase`.
- `scons/common.py:flash_run` maps `flash` type → Tcl proc name.

## 8. Python client

`projects/bootloader/scripts/` — `main.py` + per-command modules
(`flash_application`, `jump_application`, `ping_application`, `bootloader_id`), a
`can_datagram` chunkifier, a `crc32` implementation, and a `gui.py`. The CRC here
is an MSB-first, word-byte-swapped variant with a final XOR (see doc 02 for why
that matters).
