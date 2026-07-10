# 02 — Challenges & Known Problems

Grouped by theme and tagged by severity. Severity is about *getting a robust,
reusable bootloader shipped*, not just "is it a bug."

- 🔴 **Blocker** — prevents correct operation today.
- 🟠 **Important** — latent / will bite as soon as conditions change.
- 🟡 **Minor** — cleanup, correctness-in-principle, or polish.
- 🧩 **Portability** — specific obstacle to the "drop-in module" goal (doc 04).

---

## A. Memory mapping & scaling

### A1 🔴 The memory maps are copy-pasted and overlap
All three `default/memory_map.ld` files reuse the **L433 origins**
(`0x08008000 / 0x08018000 / 0x0802A000 / 0x0803C000`) and only edit the `LENGTH`s.
On the bigger parts the regions massively overlap and the declared sizes are
fiction:

| Chip | Flash | "App Active" declared | Real gap before staging |
|------|-------|-----------------------|-------------------------|
| L433CCU6 | 256K | 80K | 72K (overlap) |
| L4P5VET6 | 512K | 120K | 72K (overlap) |
| L496RGT6 | 1M | 256K | 72K (overlap) |

GNU `ld` only errors if a *section* overflows, so the bad sizes pass silently and
`_app_active_size` is wrong. Downstream effects: `boot_verify_flash_memory()`
scans past the region, and `stm_flash_fs_storage` computes a start address **past
the end of flash**.

### A2 🔴 The largest app does not fit
Steering builds to ~421 KB and targets the **L4P5VET6 (512 KB)**. With A/B staging
you'd need ~842 KB — impossible. Even a single declared 120 KB slot is ~3.5× too
small. This is the headline reason for the single-app decision (doc 03).

### A3 🟠 Wrong flash page size on L4+ parts
The L4P5 is an STM32**L4+** device; its real page is 4 KB (dual-bank) or 8 KB
(single-bank), not the `_flash_page_size = 2K` the script claims. The bootloader
also hardcodes `BOOTLOADER_PAGE_BYTES = 0x800` and `NUM_FLASH_PAGES = 128`. Erase
granularity must come from the chip, or erases corrupt neighbouring data.

### A4 🟠 Size constants disagree across three files
`memory_map.ld` LENGTHs, `device_params.cfg`, and `bootstrap_memory_map.h` (x86)
all carry independent copies of the region sizes and disagree. There must be a
single source of truth.

---

## B. DFU correctness

### B1 🔴 Three different CRC-32s that don't agree
- Bootstrap image check: reflected **zlib** CRC32 (poly `0xEDB88320`, final XOR).
- Bootloader chunk check: STM32 **hardware** CRC (non-reflected, output inversion
  disabled, **no** final XOR).
- Python client: MSB-first, word-byte-swapped, **with** final XOR.

The client applies a final inversion the HAL config does not, so client CRCs won't
match the node → every chunk fails `CRC_MISMATCH_BEFORE_WRITE`. There must be one
specified algorithm with a host↔target round-trip test.

### B2 🔴 RX filter accepts everything → out-of-bounds dispatch
`boot_can_init` sets an accept-all mask, and `main` feeds *every* frame to
`bootloader_run` → `arbitration_id_handle[msg->id]`. That table has 38 entries;
any normal system-CAN ID (up to 2047) is an out-of-bounds read, and an unknown ID
yields an **uninitialized** datagram. In-range non-command IDs map to
`UNINITIALIZED` and drive the FSM to `FAULT` → `NVIC_SystemReset()`. On a live bus
this resets constantly. Need: hardware filter to the bootloader ID range + an
explicit bounds/known-ID check.

### B3 🔴 Loopback mode is on by default
`BOOTLOADER_LOOPBACK_TESTS_ENABLE` defaults to `1`, wired into
`can_settings.loopback = true`, so the CAN peripheral never hears the real bus and
self-tests run on boot. Must be off for any real flashing.

### B4 🟠 Jump to app doesn't set MSP/VTOR
`bootloader_jump_app`'s inline asm branches to the app reset vector without setting
the stack pointer or `VTOR` and without disabling IRQs — exactly the "funky jump"
flagged from FW15. Should reuse the bootstrap's correct `jump_to`.

### B5 🟠 No per-node targeting
`target_nodes` is parsed but the "am I a target?" check is commented out, so
multi-node selective flashing (a core requirement) doesn't work and every node
acts on every command.

---

## C. Robustness gaps vs. the design

### C1 🟠 MPU protection is dead code
`protect_bootloader_memory()` is never called; `init.Size` is given a raw byte
count instead of an encoded `MPU_REGION_SIZE_*`; and `HAL_MPU_Enable` runs before
the region is configured.

### C2 🟠 No independent watchdog
The design calls for the IWDG to recover from hangs; only a SysTick inactivity
timeout exists. A wedged erase/write or stuck poll has no recovery.

### C3 🟠 Entry into the bootloader needs provisioned `crc.txt`
The bootstrap only enters the bootloader if `midFS` already contains a `crc.txt`
matching the bootloader region; otherwise it silently jumps to the app or halts.
Nothing visibly writes that file, so a fresh board can't reach the CAN bootloader
without an extra step. The redesign removes this dependency.

### C4 🟡 Self-update of the bootloader is undefined
"DFU the bootloader over CAN" is a goal, but a running bootloader can't erase the
flash it executes from, and there's no staging/apply path. Doc 03 proposes using
the bootstrap to apply a staged image.

---

## D. Smaller correctness issues

| ID | Issue | File |
|----|-------|------|
| D1 🟡 | `error_buffer[1] = (error << 8)` always 0; should be `>> 8`. | `bootloader_can_datagram.c` |
| D2 🟡 | `bootloader_ping`: `memset(buf, 0, sizeof(buf))` zeroes 4 bytes (pointer), `bytes_written` never incremented so completion never fires, fixed 8-byte tail copy. | `bootloader_packet_handler.c` |
| D3 🟡 | `boot_align_to_32bit_words` logic inverted (returns when padding needed). Unused. | `bootloader_crc32.c` |
| D4 🟡 | `bootloader_init` overwrites `error` each sub-init, masking failures. | `bootloader.c` |
| D5 🟡 | FIFO1 RX path is dead — the single filter routes everything to FIFO0. | `bootloader_can.c` |
| D6 🟡 | `stm_flash_app_staging` referenced in `common.py` but not defined in the Tcl. | `scons/common.py` |

---

## E. Portability coupling (🧩 — the big one for reuse)

These aren't bugs; they're the things stopping the bootloader from being a
copy-paste module. See doc 04 for the target boundary.

| ID | Coupling | Why it blocks reuse |
|----|----------|---------------------|
| E1 | Direct `stm32l4xx_hal_*` calls in `bootloader_can.c`, `_flash.c`, `_crc32.c`, `_mpu.c`. | Hard-wires the module to STM32L4 + CubeHAL. A different MCU/HAL (or LL drivers) can't reuse it. |
| E2 | Bootstrap depends on `midFS`. | Drags a whole filesystem library into any host repo. |
| E3 | Region addresses come from this repo's specific linker-symbol names (`_app_active_start`, …). | A new repo must reproduce the exact symbol contract, undocumented. |
| E4 | Build/flash logic baked into this repo's SCons (`arm.py`, `build_presets.json`, `stm32_flash_procs.tcl`). | Not portable to a Make/CMake repo; no standalone build. |
| E5 | CAN ID scheme assumes this repo's `SYSTEM_CAN_DEVICE_*` partitioning. | A host repo with a different CAN ID map needs a clean, parameterized integration point. |
| E6 | Hardcoded flash geometry (`0x800`, `128 pages`, 256 KB). | Won't move to another flash size/part without edits. |

**Takeaway:** the redesign should split a *portable core* (state machine, protocol,
CRC contract, partition math, config logic) from a thin *port layer* (CAN, flash,
timer, watchdog, jump, MPU) that each repo implements — and document the linker
symbol + CAN ID contract explicitly.
