# 03 — Memory Mapping (Redesign)

How we partition flash, why, and how to make one scheme work on any STM32. This
supersedes the layout in doc 01.

---

## 1. Constraints that drive the layout

1. **Reset entry is fixed.** On reset the core fetches `MSP` and the reset vector
   from `0x08000000`. Whatever lives there is the permanent first stage. → the
   **bootstrap** owns the base.
2. **Every boundary must be flash-page aligned.** You erase a whole page at a time,
   so region edges must land on page boundaries. Page size is **per chip**
   (2 KB on L43x/L49x, 4 KB/8 KB on L4+).
3. **Each stage must relocate `VTOR`** to its own vector table and set `MSP`
   before running (interrupts and the stack must point at the right place).
4. **No hardware locking (v1).** No MPU/WRP write-protect (decided out). The bootstrap
   is permanent and SWD-reflashable, so it's the safety net; we rely on careful flash
   addressing, not write-protect. Keep the stages at the low end of flash, page-aligned.
5. **Dual-bank parts** (L496, L4P5) can swap banks, but we deliberately **don't**
   rely on bank-swap — it's MCU-specific and works against portability. Treat
   flash as a flat, single-bank space.

---

## 2. Chip facts

| Chip | Flash | RAM | Page | Bank | Used by |
|------|-------|-----|------|------|---------|
| STM32L433CCU6 | 256 KB | 64 KB | 2 KB | single | most boards |
| STM32L4P5VET6 | 512 KB | 320 KB | 4 KB / 8 KB | dual / single | **steering** (~421 KB app) |
| STM32L496RGT6 | 1 MB | 320 KB | 2 KB | dual | larger boards |

> ⚠️ Confirm the L4P5 page size against RM0432 and the configured bank mode before
> implementing; today's scripts wrongly assume 2 KB.

---

## 3. The model: fixed small stages, app = remainder, config at the top

Only two numbers vary per chip: **`FLASH_KB`** and **`PAGE_KB`**. Everything else
is computed; the application absorbs all leftover flash. The **config sits in the
last page of flash** so the two stages stay contiguous at the base and the config
lives at a fixed address regardless of app size.

```
┌──────────────────────────┐  base = 0x08000000   ─┐
│ Bootstrap   (fixed 8K)   │  reset entry           │ permanent,
├──────────────────────────┤                        │ SWD-reflashable
│ Bootloader  (fixed 32K)  │  CAN DFU               ─┘ (no HW lock)
├──────────────────────────┤
│                          │
│ Application (remainder)  │  ← all the rest
│                          │
├──────────────────────────┤
│ BootConfig  (1 page)     │  CRC32s + metadata (see §5)
└──────────────────────────┘  end of flash
```

```
APP_SIZE      = FLASH_KB - BOOTSTRAP_KB - BOOTLOADER_KB - CONFIG_KB
CONFIG_KB     = PAGE_KB                             ; exactly one page (no A/B)
APP_ORIGIN    = base + BOOTSTRAP_KB + BOOTLOADER_KB
CONFIG_ORIGIN = base + FLASH_KB - CONFIG_KB
```

All of these are computed by the linker autogen (§7) from `bootloader_user_config.h`.

### Why these choices

- **Single app, no staging** — see doc 02 §A2; rollback can't be afforded on the
  512 KB part and is wasteful elsewhere. Anti-brick is provided by CRC-before-jump
  plus the protected bootloader, not by a spare slot.
- **Keep a tiny bootstrap** — it's the permanent thing that lets us (in v2)
  field-update the *bootloader* over CAN and still recover. In v1 it needs no CAN, no
  FS — just CRC, vector-table validation, and a jump (the flash-copy apply path is v2).
  It links only the `common/` lib, which keeps it ~8 KB.
- **`BootConfig` at the top, not a filesystem** — removes the `midFS` dependency
  (a portability win). A flat CRC'd struct is enough; littleFS-style wear-leveling
  is overkill because config writes are rare and bounded (provisioning + once per
  DFU). See §5 and doc 05.
- **No FS region** — nothing but the old bootstrap used it.

---

## 4. Proposed per-chip layouts

With `bootstrap = 8K`, `bootloader = 32K`, `config = 1 page` (config at top):

| Chip | Bootstrap | Bootloader | **Application** | BootConfig | Fits 421K? |
|------|-----------|-----------|-----------------|------------|-----------|
| L433CCU6 (256K, 2K) | `08000000` (8K) | `08002000` (32K) | `0800A000` → `0803F800` = **214K** | `0803F800` (2K) | n/a |
| L4P5VET6 (512K, 4K) | `08000000` (8K) | `08002000` (32K) | `0800A000` → `0807F000` = **468K** | `0807F000` (4K) | ✅ ~47K margin |
| L496RGT6 (1M, 2K)  | `08000000` (8K) | `08002000` (32K) | `0800A000` → `080FF800` = **982K** | `080FF800` (2K) | ✅ |

> The steering margin is now ~47 KB (~10%). Confirm the real `.bin` size with
> `arm-none-eabi-size` before committing.
>
> Note: in L4P5 **8 KB-page single-bank** mode, one config page = 8 KB (app = 464 KB).
> The autogen handles this parametrically from `BL_FLASH_PAGE_SIZE_B`.

---

## 5. Config region (`BootConfig`)

A flat, CRC'd struct in the last page of flash — **not** a filesystem. It holds
CRC32s + sizes for **both** the bootloader and the app, which are the gate for each
jump:

- **Bootstrap → bootloader:** CRC the bootloader region, compare to `bl_crc32`.
- **Bootloader → app:** CRC the app over `app_size`, compare to `app_crc32`.

**Exactly one page, no A/B redundancy** (decided out). A torn write just fails the
CRC → config treated blank → "no app, stay in DFU" (safe fallback). Writable by the
bootloader, read-only-readable by the app. The struct is **designed to grow** (fixed
prologue + append-only body + size-delimited CRC) so v2 can add fields without
breaking v1 readers.

The full struct layout, the forward-compat rules, and the app-side read API live in
**[doc 05](05_config_and_linker_autogen.md)**.

---

## 6. Boot, validate & update flows

### Normal boot
```
reset
 └▶ bootstrap
     ├─ (v2 only) if config.pending_bl_update && CRC(staged) ok:
     │     erase bootloader region, copy staged→bootloader, verify, clear flag
     ├─ validate bootloader vector table (SP in RAM, reset vec in BL region)
     │     + CRC vs config.bl_crc32 when present
     └─ jump_to(bootloader)         ; set MSP + VTOR
          └▶ bootloader
              ├─ bring up CAN, wait N s for a client
              ├─ (client commands → DFU / query / update…)
              └─ on timeout / JUMP_APP:
                    if app_present && CRC(app, app_size)==config.app_crc32:
                        jump_to(app)   ; set MSP + VTOR
                    else stay in bootloader
```

### App DFU
Stream into the app region, CRC each page, and **on success write
`app_size`/`app_crc32`/`app_present` into config**. A failed/partial flash leaves
`app_present`/CRC not matching → the bootloader refuses to jump and waits. No brick.

### Bootloader self-update (v2 — deferred; why the bootstrap exists)
> **v1 is SWD-only for the bootloader** (doc 07 §9). The flow below is the v2 design
> the bootstrap is *built to allow* — it's not implemented in v1, but it's why the
> permanent bootstrap and the reserved BootConfig fields exist.

A running bootloader can't erase itself. So:
1. Client streams the **new bootloader image into the app region** (temp).
2. Bootloader CRC-checks it, sets `pending_bl_update + bl_staging_addr/size/crc`,
   resets.
3. Bootstrap (runs from its own region) erases the bootloader region, copies the
   staged image in, verifies, clears the flag, jumps.
4. The app slot is now dirty → re-flash the app.

This keeps the bootstrap tiny and CAN-free while still allowing CAN bootloader
updates with a guaranteed fallback.

---

## 7. Making it chip-agnostic in the build

The three hand-edited `memory_map.ld` files are replaced by a **linker autogen**:
the host edits `bootloader_user_config.h` (geometry knobs only) and a script emits
the `.ld`. One source of truth, consumed by both the C side and the linker.

```c
/* bootloader_user_config.h — the ONLY file a new board edits */
#define BL_FLASH_BASE         0x08000000U
#define BL_FLASH_SIZE_KB      512U
#define BL_FLASH_PAGE_SIZE_B  4096U   /* erase granularity for THIS part */
#define BL_BOOTSTRAP_SIZE_KB  8U
#define BL_BOOTLOADER_SIZE_KB 32U
/* CONFIG = exactly one page (no A/B); APP = remainder. */
```

The firmware reads addresses from the **generated linker symbols** (never
hardcoded constants) and scalar knobs (page size, timeouts, CAN) from this header
directly — so the same C compiles for every part. The autogen script, the exact
symbol names it emits, and how SCons/Make invoke it are specified in
**[doc 05](05_config_and_linker_autogen.md)**.

[cvra]: https://github.com/cvra/can-bootloader
