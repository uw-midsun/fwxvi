# 04 — Portable Module Design

The goal that shapes everything: **a bootloader you can copy-paste into any future
repo** (drive-inverter FW, a new board, a side project) and bring up on a new
STM32 by filling in a small port layer — without dragging in midFS, our SCons
setup, or STM32L4-specific assumptions.

---

## 1. Layering

Split the code into four layers with a hard dependency rule: **inner layers never
include outer ones.**

```
┌─────────────────────────────────────────────────────────────┐
│ 4. Client (host)      Python CLI/GUI — speaks the protocol    │
├─────────────────────────────────────────────────────────────┤
│ 3. Build glue         linker template, SCons/Make/CMake hooks │
├─────────────────────────────────────────────────────────────┤
│ 2. Port layer         CAN, UART, flash, timer, watchdog, jump │  ← per repo / per MCU
│    (bootloader_port.h: prototypes only)                       │
├─────────────────────────────────────────────────────────────┤
│ 1. Portable core      state machine, datagram protocol,       │  ← copy as-is,
│    CRC contract, partition math, config region logic          │     never edit
└─────────────────────────────────────────────────────────────┘
```

- **Layer 1 (core)** has *zero* vendor includes. No `stm32l4xx_hal.h`, no `midFS.h`,
  no `gpio.h`. It only `#include "bootloader_port.h"` and standard headers. This is
  the part that gets reused unchanged.
- **Layer 2 (port)** is what a new repo writes. It's the only place HAL/LL/register
  code lives.
- **Layers 3–4** are conveniences, kept loosely coupled so a Make-based repo can
  ignore our SCons.

This is the single most important refactor for reuse: today the core logic and the
HAL calls are interleaved in the same files (doc 02 §E1).

---

## 2. Submodule vs. host repo

The module is a **git submodule**. The split is: *everything invariant lives in the
submodule; everything board-specific lives in the host repo.* The host never edits
submodule files (so it can pull updates cleanly).

```
the-submodule/  (ms-bootloader, added via `git submodule add`)
│  Everything in Layer 1 is a small static lib; bootstrap & bootloader are two
│  "apps" that link different subsets (so bootstrap stays ~8 KB).
├── common/                    # shared lib — linked by BOTH bootstrap & bootloader
│   ├── bootloader_config.c/.h     # BootConfig read/write, app-readable (no A/B)
│   ├── bootloader_crc32.c/.h      # software CRC (the spec'd algorithm)
│   ├── bl_jump.c/.h               # MSP/VTOR jump + vector-table sanity
│   ├── bl_status.h                # BlStatus codes
│   └── bootloader_port.h          # THE raw-HAL porting interface (prototypes only)
├── bootloader/                # DFU-engine app — links common + transport
│   ├── bootloader.c/.h            # state machine + gateway routing
│   ├── bl_datagram.c/.h           # datagram (de)serialize, verify, ACK/NACK
│   └── bl_dfu.c/.h                # datagram → flash (direct-to-app, lazy erase)
├── bootstrap/                 # first-stage app — links common ONLY (v1)
│   ├── bootstrap.c/.h             # validate bootloader (vector + CRC) → jump
│   └── bl_entry_shim.c/.h         # ~50-line app-side jump-to-BL listener (doc 07 §7)
├── transport/                 # submodule code (owns the wire framing, doc 07 §11)
│   ├── bl_transport.h             # the vtable (init/poll/send_datagram)
│   ├── can/                       # role-ID frame scheme  → calls bl_port_can_*
│   └── uart/                      # network_buffer + BlPacket FSM → calls bl_port_uart_*
├── port/                      # Layer 2 — reference ports the host can pick from
│   └── stm32l4/{port_can,port_uart,port_flash,port_time,port_jump}.c
├── linker/
│   ├── memory_map.ld.in           # template consumed by the autogen (doc 05)
│   └── sections_common.ld
├── tools/
│   └── generate_linkerscript.py   # the linker autogen (doc 05)
├── client/                    # Layer 4 — host tooling (Python CLI/GUI)
└── docs/

# Linkage: bootstrap = common. bootloader = common + bl_datagram + bl_dfu + transport.
# app = common (config-read) + bl_entry_shim. v2 bootstrap-DFU just adds transport +
# bl_datagram to bootstrap's link — no restructure, since they're already separate libs.
#
# `transport/{can,uart}` are SUBMODULE code — they own doc 07 §11's framing and call
# DOWN to the host's raw-HAL `bl_port_*`. A host repo never re-implements a transport;
# it implements only the port (§4).

host-repo/
├── bootloader_user_config.h   # ← THE file you edit (geometry, CAN, timeouts)
├── port/<mcu>/                 # ← your port impl (or symlink a reference port)
└── (build glue: SCons/Make wires the autogen + submodule sources)
```

A new repo: add the submodule, write `bootloader_user_config.h`, provide a
`port/<mcu>/` (start from a reference port), wire the autogen into the build. No
submodule files are touched.

---

## 3. CRC: one algorithm, shared by host and target

Pick **one** CRC-32 and use it everywhere (image check, chunk check, config check),
on both host and target. Recommendation: standard reflected CRC-32 (zlib) computed
**in software** in the core, so it's identical on every MCU and matches Python's
`zlib.crc32` / `binascii.crc32` exactly. Drop the dependence on the STM32 hardware
CRC peripheral (it's a portability + correctness trap — see doc 02 §B1). Add a
round-trip test: host CRC of a blob == target CRC of the same blob.

---

## 4. The port interface (`bootloader_port.h`)

The contract a new repo implements. Keep it small — this is the entire surface area
of "porting to a new MCU."

```c
// ---- CAN (raw frames; transport/can sits above this) ----------------------
BlStatus bl_port_can_init(uint32_t bitrate_kbps);
// Non-blocking: return BL_OK if a frame was read, BL_EMPTY if none.
BlStatus bl_port_can_rx(uint32_t *id, uint8_t data[8], uint8_t *dlc);
BlStatus bl_port_can_tx(uint32_t id, const uint8_t *data, uint8_t dlc);

// ---- UART (raw bytes; transport/uart sits above this) ---------------------
BlStatus bl_port_uart_init(uint32_t baud);
// Non-blocking: read up to *len bytes, write back the count actually read.
BlStatus bl_port_uart_rx(uint8_t *buf, uint32_t *len);
BlStatus bl_port_uart_tx(const uint8_t *buf, uint32_t len);

// ---- Flash ----------------------------------------------------------------
uint32_t bl_port_flash_page_size(void);          // erase granularity
BlStatus bl_port_flash_erase(uint32_t addr, uint32_t len);
BlStatus bl_port_flash_write(uint32_t addr, const uint8_t *buf, uint32_t len);
// Reads may just be memcpy from mapped flash, but keep it abstract.
BlStatus bl_port_flash_read(uint32_t addr, uint8_t *buf, uint32_t len);

// ---- Time / watchdog ------------------------------------------------------
uint32_t bl_port_now_ms(void);
void     bl_port_watchdog_init(uint32_t timeout_ms);
void     bl_port_watchdog_kick(void);

// ---- Control flow ---------------------------------------------------------
void     bl_port_jump(uint32_t vector_table_addr);  // set MSP+VTOR, branch
void     bl_port_reset(void);
// (No MPU/WRP region locking in v1 — decided out. The bootstrap is the safety net:
//  it's permanent and SWD-reflashable, so we rely on careful flash addressing, not
//  hardware write-protect.)
```

### Linker symbol contract (the other half of the port)

The core reads geometry from these symbols; the **linker autogen emits them** from
`bootloader_user_config.h` (doc 05), so the host never hand-writes them:

```
_flash_start, _flash_size, _flash_page_size
_bootstrap_start, _bootstrap_size
_bootloader_start, _bootloader_size
_config_start,    _config_size
_app_start,       _app_size
_sram_start,      _sram_size
```

### CAN ID contract

`transport/can` needs its reserved IDs + this board's node id. Parameterize, don't
hardcode this repo's `SYSTEM_CAN_DEVICE_*` (the module has **no** system_can
dependency — doc 07 §3):

```c
#define BL_ENTER_ID      <low/high-prio>  // app-facing jump-to-BL (doc 07 §11)
#define BL_XFER_ID_BASE  <repo chooses>   // FRAGMENT/RESPONSE/ACK = BASE+0..2
#define BL_NODE_ID       <0 = from BootConfig.board_id>   // for 64-bit mask targeting
```

So a host repo with a different CAN map only edits these macros, not the protocol.

---

## 5. What stays in the host repo vs. the module

| Concern | Module provides | Host repo provides |
|---------|-----------------|--------------------|
| State machine, protocol, DFU, config | ✅ core | — |
| CRC algorithm | ✅ software impl | — |
| CAN / flash / timer / jump | interface + STM32L4 example | a `port/<mcu>/` impl |
| Memory map | parametric template | `FLASH_KB` / `PAGE_KB` for the part |
| Board ID / project name | config schema | values written at provision/flash time |
| Build | example SCons + Make snippet | wire into its own build |
| Client | Python CLI/GUI | (just run it) |

---

## 6. Integration checklist (new repo, e.g. drive inverter / MPPT)

1. `git submodule add <ms-bootloader> third_party/ms-bootloader`.
2. Write **`bootloader_user_config.h`** — flash geometry (`BL_FLASH_*`,
   `BL_*_SIZE_KB`; config is a single page), `BL_CAN_BITRATE_KBPS`, `BL_ENTER_ID`,
   `BL_XFER_ID_BASE`, node-ID source. (Timeouts are hardcoded in core, not here.)
3. Provide `port/<mcu>/` — implement the ~12 `bl_port_*` functions (start from a
   reference port; only needed if your MCU isn't already covered).
4. Wire the build: run `tools/generate_linkerscript.py` (reads
   `bootloader_user_config.h` → `.ld`), then compile `common/` + `bootloader/` +
   `transport/` + `bootstrap/` + your `port/` against it. Build the app with the
   generated app `.ld` (it links `common/` config-read + `bootstrap/bl_entry_shim`).
5. Run the host round-trip test (CRC parity) and a loopback bring-up.
6. Provision board ID / name into the `BootConfig` region.

For an MCU that's already covered by a reference port, steps 1–2 + 4 are the whole
job — that's the module goal met.

---

## 7. Open questions

- **Bootstrap size** — target **8 KB** (midFS is gone, common lib is dep-light); pin
  the strict number once it builds. Treated as a soft target for now.
- **Bitrate negotiation** — fixed (user-config knob) everywhere, or discover? Fixed is
  simpler and matches the bus; keep fixed unless a repo needs otherwise.
- **Client language** — Python now; a Rust client is desirable later but out of
  scope. Keep the protocol simple enough that a second client is easy.
- **L4P5 page size / bank mode** — must be pinned down before the flash port is
  trusted (doc 03 §2).
