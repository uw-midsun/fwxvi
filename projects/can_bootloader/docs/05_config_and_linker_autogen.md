# 05 — `BootConfig` & Linker Autogen

The two concrete artifacts that turn the bootloader into a drop-in submodule:

1. **`bootloader_user_config.h`** — the single file a host repo edits.
2. **The linker autogen** — turns that file into the `.ld`, so geometry has one
   source of truth.
3. **`BootConfig`** — the flat, CRC'd flash region (top of flash) read/written
   without a filesystem.

---

## 1. `bootloader_user_config.h` — the user interface

Lives in the **host repo** (not the submodule). Plain integer `#define`s only — no
function-like macros, no casts — so both the C compiler and the autogen can read
it. Macros may reference earlier macros.

```c
#pragma once
/* ============================ FLASH GEOMETRY ============================ */
#define BL_FLASH_BASE          0x08000000U
#define BL_FLASH_SIZE_KB       512U        /* total flash on THIS part        */
#define BL_FLASH_PAGE_SIZE_B   4096U        /* erase granularity on THIS part  */

#define BL_BOOTSTRAP_SIZE_KB   8U           /* target; page-aligned            */
#define BL_BOOTLOADER_SIZE_KB  32U          /* fixed, page-aligned             */
/* CONFIG = exactly ONE page (no A/B redundancy). APP = everything in between; */
/* both computed by the autogen.                                              */

/* ============================== SRAM =================================== */
#define BL_SRAM_BASE           0x20000000U
#define BL_SRAM_SIZE_KB        320U

/* ============================== CAN ==================================== */
#define BL_CAN_BITRATE_KBPS    500U
#define BL_ENTER_ID            0x010U        /* low ID = high prio; app-facing  */
#define BL_XFER_ID_BASE        0x100U        /* FRAGMENT/RESPONSE/ACK = BASE+0..2 */
#define BL_NODE_ID             0U           /* 0 = read from BootConfig.board_id */
```

> **Timeouts are not user-config.** The round-4 decision is that the 5 s entry window,
> ~25 ms inter-frame timeout, and ~2 s IWDG period are **hardcoded constants in the
> core** (only the CAN bitrate is a knob). They are intentionally absent above.

> **Constraints worth stating in the file header:** sizes must be multiples of
> `BL_FLASH_PAGE_SIZE_B`; `BOOTSTRAP + BOOTLOADER + CONFIG < FLASH`; the autogen
> asserts these and fails the build with a clear message if violated.

---

## 2. Linker autogen (`tools/generate_linkerscript.py`)

### Contract

```
in:   bootloader_user_config.h
out:  <build>/memory_map.ld     (MEMORY {} + exported symbols)
```

The static `sections_*.ld` templates stay as-is (they just
`REGION_ALIAS("_flash_target_region_", <REGION>)` + include `sections_common.ld`).
Only `memory_map.ld` is generated. The same generated file serves the bootstrap,
bootloader, **and** application links — the app's script aliases the `APP` region.

### What it does

1. Parse `#define BL_*` lines (regex), strip `U`/`UL` suffixes, evaluate the RHS as
   a restricted integer expression (`+ - *`, hex, prior `BL_*`).
2. Compute the layout (doc 03 §3): `APP_ORIGIN`, `APP_SIZE`, `CONFIG_ORIGIN`, etc.
3. Validate (page alignment, no overflow, app > 0).
4. Emit `MEMORY {}` and the symbol exports below.

### Emitted symbols (the firmware's contract — doc 04 §4)

```
_flash_start, _flash_size, _flash_page_size
_bootstrap_start, _bootstrap_size
_bootloader_start, _bootloader_size
_app_start,    _app_size
_config_start, _config_size
_sram_start,   _sram_size,  _estack
```

### Generated `memory_map.ld` (shape)

```ld
/* AUTO-GENERATED from bootloader_user_config.h — do not edit. */
MEMORY {
  FLASH      (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
  BOOTSTRAP  (rx)  : ORIGIN = 0x08000000, LENGTH = 8K
  BOOTLOADER (rx)  : ORIGIN = 0x08002000, LENGTH = 32K
  APP        (rx)  : ORIGIN = 0x0800A000, LENGTH = 468K
  CONFIG     (rw)  : ORIGIN = 0x0807F000, LENGTH = 4K   /* one page */
  RAM        (rwx) : ORIGIN = 0x20000000, LENGTH = 320K
}
_app_start = ORIGIN(APP);  _app_size = LENGTH(APP);
_config_start = ORIGIN(CONFIG);  _config_size = LENGTH(CONFIG);
/* …rest of the symbols… */
```

### Build wiring

- **SCons (this repo):** a builder runs the script before link; `platform/arm.py`
  uses the generated path in `-T`. Replaces the per-chip hand-edited maps.
- **Make/CMake (other repos):** a pre-link rule: `python generate_linkerscript.py
  bootloader_user_config.h -o build/memory_map.ld`.

> **Why generate instead of `cpp`-preprocess the `.ld` directly?** `ld` expressions
> don't accept C integer suffixes (`512U`) and the math/validation we want is
> awkward in linker syntax. A tiny Python generator is clearer, validates inputs,
> and emits identical symbols for C and the linker. (A `cpp` template remains a
> fallback if a repo can't run Python at build time.)

---

## 3. `BootConfig` (flash, top of flash)

### Why not littleFS

Config writes are rare and bounded (provisioning once + one write per successful
DFU), so wear-leveling buys nothing and littleFS would cost multiple pages plus a
dependency. A single CRC'd struct in **one** page gives integrity for ~free. No A/B
redundancy (decided out): a torn write just fails the CRC → the bootloader treats
config as blank → "no valid app, stay in DFU," which is the safe fallback.

### Layout (`bootloader_config.h`, in the submodule) — designed to grow

Forward-compatibility is built in so v2+ can add fields **without** breaking a v1
reader or shifting the CRC. Three rules:

1. **Fixed prologue** (`magic`, `schema_version`, `size`) at offset 0 — its layout
   *never* changes across versions.
2. **Append-only body** — new versions only *append* fields after the existing ones;
   never reorder/resize/remove. So every existing field keeps its byte offset forever.
3. **Size-delimited trailing CRC** — `config_crc32` is the last 4 bytes of the
   `size`-long blob, located at `base + size - 4` (not at a fixed struct offset). A
   reader of *any* version finds and verifies it via `size`, even for a newer writer.

```c
#define BOOTCONFIG_MAGIC   0x424F4F54U   /* "BOOT" */
#define BOOTCONFIG_SCHEMA  1U            /* bump when appending fields */

typedef struct {
  /* ---- fixed prologue: NEVER reorder/resize across schema versions ---- */
  uint32_t magic;             // BOOTCONFIG_MAGIC
  uint16_t schema_version;    // schema the writer used
  uint16_t size;              // total valid bytes [magic .. end of config_crc32]

  /* ---- schema-1 body: append-only (v2+ adds fields AFTER, before crc) -- */
  uint16_t board_id;          // 0..63 — IS the bootloader node_id (doc 07 §3)
  char     board_name[32];    // optional, human-friendly
  char     project_name[32];  // current app, e.g. "drive_inverter"
  char     git_hash[16];      // e.g. "0bdfdd8-dirty"
  uint8_t  fw_version_major;
  uint8_t  fw_version_minor;

  uint32_t bl_size;           // bootloader image size  ─┐ checked by bootstrap
  uint32_t bl_crc32;          // bootloader image CRC32  ─┘ before jump→bootloader
  uint32_t app_size;          // app image size  ─┐ checked by bootloader
  uint32_t app_crc32;         // app image CRC32  ─┘ before jump→app
  uint8_t  app_present;       // 0 = no valid app

  /* ===== v2+ fields get appended right here, BOOTCONFIG_SCHEMA bumped ===== */
  /*       (e.g. bl self-update: pending_bl_update, bl_staging_{addr,size,crc}) */

  uint32_t config_crc32;      // CRC32 over the first (size - 4) bytes
} BootConfig;                 // this struct == the CURRENT schema's view
```

- **Read (any version):** check `magic`; sanity-check `size` (≤ page); read the CRC at
  `base + size - 4`; verify CRC32 over `[base, base+size-4)` using the *raw bytes* (no
  field interpretation needed). Then interpret fields up to
  `min(my_schema, file_schema)`; ignore any trailing bytes a newer writer added (still
  CRC-covered). If `magic`/CRC fail → config blank → "no valid app, stay in DFU."
- **Write:** erase the one config page, write the struct (with its `size`/`schema`),
  recompute the trailing CRC, read back + verify. No A/B slot.
- A v1 reader of a v2-written config still validates (CRC over `size` bytes) and reads
  every schema-1 field at its unchanged offset — that's the whole point of the design.

### APIs (`bootloader_config.c`, in the submodule)

```c
BlStatus bl_config_read(BootConfig *out);        // app + bootloader (read path only links CRC+read)
BlStatus bl_config_write(const BootConfig *in);   // bootloader only (pulls in flash erase/write)
```

- The **app** links just the read path and includes `bootloader_config.h`, so it
  can learn its own `board_id` / `project_name` / `git_hash` at runtime.
- The **bootloader** links read + write.
- Address comes from the `_config_start` linker symbol — no hardcoded address.

---

## 4. Who writes the CRCs (provisioning)

| Field | Written by | When |
|-------|-----------|------|
| `bl_size`, `bl_crc32` | flash/provision tool (`tools/provision_config.py` or an OpenOCD step) | when the bootloader is SWD-flashed — it computes zlib CRC of the `.bin` and writes a config slot |
| `app_size`, `app_crc32`, `app_present`, `project_name`, `git_hash` | the **bootloader** | after a successful CAN DFU (it already has the streamed size + CRC) |
| `board_id`, `board_name` | provision tool / `boot-update-id` command | at provision time / on request |

This removes the old `crc.txt`-in-`midFS` chicken-and-egg (doc 02 §C3): the
bootstrap reads `bl_crc32` straight from the config region, and there's no
filesystem in the loop.

---

## 5. What this buys the "drop-in" goal

- A new board's entire flash story is **one header**.
- The autogen guarantees the C side and the linker agree.
- The config has no library dependency and is reachable from both the bootloader
  (R/W) and the app (R).
- The submodule ships the generator, templates, core, and a reference port; the
  host adds a header + (maybe) a port. Nothing in the submodule is hand-edited.
