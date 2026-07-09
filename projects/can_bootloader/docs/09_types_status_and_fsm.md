# 09 — Types, Status Codes & State Machine

The type contract the rest of the module is built against: the internal `BlStatus`
error type, the **wire-level** NACK/ACK status byte, the new DFU state machine, and a
handful of clarifications that docs 03–07 left "to implementation" but that the early
phases (P0–P2) actually need pinned. Where this doc and 01–07 disagree, **07 still
wins** on protocol; this doc only *adds* the type/FSM layer beneath it.

Everything is `bl_` / `Bl*` / `BL_`. Comment/diagram style matches `projects/fota/`
(file header block, `@defgroup`, ASCII box tables, per-field `/**< */`).

---

## 0. Two distinct "status" concepts (don't conflate them)

| | `BlStatus` | `BlNackCode` |
|---|---|---|
| Scope | **internal** return type, every core function | **on the wire**, 1 byte in `BL_ACK`/`BL_NACK` |
| Width | `enum` (int) | `uint8_t` |
| Audience | firmware call stack | the Python client |
| Analog | FOTA's `FotaError` | (FOTA had none — it's new) |

A node maps an internal `BlStatus` failure to the coarser `BlNackCode` it reports to
the client. They are deliberately separate: the internal enum can grow freely without
changing the wire protocol, and the wire byte stays stable + small.

---

## 1. `BlStatus` — internal error type (`common/bl_status.h`)

The portable core carries **no dependency on the host repo's `StatusCode`** and **no
dependency on `FotaError`** — it defines its own type so the submodule compiles in any
repo (doc 04 §1). Modelled on `FotaError` (`projects/fota/inc/fota_error.h`), renamed.

```c
/**
 * @brief   Bootloader internal status / error codes
 *
 * Returned by every core function. BL_OK (== 0) is the only success value;
 * propagate with bl_status_ok_or_return(). NOT sent on the wire — see BlNackCode.
 */
typedef enum BlStatus {
  BL_OK = 0,                  /**< Success — the only non-error value */

  /* ---- generic ---- */
  BL_ERR_INVALID_ARGS,        /**< NULL / out-of-range argument */
  BL_ERR_INTERNAL,            /**< Internal invariant violated */
  BL_ERR_TIMEOUT,             /**< Operation / inter-frame timed out */
  BL_ERR_RESOURCE_EXHAUSTED,  /**< Buffer / slot full */

  /* ---- CRC ---- */
  BL_ERR_CRC_MISMATCH,        /**< Computed CRC32 != expected */
  BL_ERR_NOT_ALIGNED,         /**< Length / address not 8-byte aligned */

  /* ---- flash ---- */
  BL_ERR_FLASH_ERASE,         /**< Page erase failed */
  BL_ERR_FLASH_WRITE,         /**< Double-word program failed */
  BL_ERR_FLASH_READ,          /**< Read-back failed */
  BL_ERR_FLASH_OUT_OF_BOUNDS, /**< Address outside the target region */
  BL_ERR_FLASH_VERIFY,        /**< Read-back != written bytes */

  /* ---- transport / datagram ---- */
  BL_ERR_CAN,                 /**< bl_port_can_* failure */
  BL_ERR_UART,                /**< bl_port_uart_* failure */
  BL_EMPTY,                   /**< Non-blocking rx: nothing available (not an error) */
  BL_ERR_INVALID_FRAGMENT,    /**< Fragment index past datagram end, bad role, etc. */
  BL_ERR_DATAGRAM_INCOMPLETE, /**< Verify before all fragments present */
  BL_ERR_DATAGRAM_TOO_LARGE,  /**< total_length > BL_MAX_DATAGRAM_SIZE */

  /* ---- DFU / state machine ---- */
  BL_ERR_INVALID_STATE,       /**< Datagram type illegal for the current state */
  BL_ERR_SEQUENCE,            /**< datagram_id out of order */
  BL_ERR_IMAGE_OVERSIZED,     /**< Stream would exceed _app_size */
  BL_ERR_NOT_TARGETED,        /**< target_node_mask doesn't include this node (skip, not fault) */

  /* ---- config ---- */
  BL_ERR_CONFIG_BLANK,        /**< magic/CRC fail → treat as no valid config */

  /* ---- jump ---- */
  BL_ERR_BAD_VECTOR_TABLE,    /**< SP not in RAM or reset vec outside region */
  BL_ERR_NO_VALID_APP,        /**< app_present == 0 or app CRC mismatch */
} BlStatus;

/**
 * @brief   Forward a failure up the stack, continue on success
 * @param   code Expression returning BlStatus
 */
#define bl_status_ok_or_return(code)   \
  do {                                 \
    BlStatus _bl_st = (code);          \
    if (_bl_st != BL_OK) {             \
      return _bl_st;                   \
    }                                  \
  } while (0)
```

> **Note `BL_EMPTY`:** the non-blocking `bl_port_can_rx`/`bl_port_uart_rx` return it
> when no data is waiting (doc 04 §4). It is *not* a fault — callers test for it
> explicitly and must not feed it to `bl_status_ok_or_return`.
>
> **Note `BL_ERR_NOT_TARGETED`:** the router uses it to *skip* a datagram whose mask
> excludes this node (doc 07 §3); it never drives a NACK or a fault.

---

## 2. `BlNackCode` — the wire status byte (`bootloader/bl_datagram.h`)

One byte, sent in the `status` field of `BL_ACK` (`[status:1][datagram_id:4][src_node:1]`,
doc 07 §11) and in a `BL_DG_NACK` payload. `BL_NACK_OK` (0) in a `BL_ACK` frame means
success; any non-zero value makes the frame a negative acknowledgement and tells the
client *why*, so it can decide retransmit vs. abort.

```
BL_ACK frame status byte:

  +--------+-------------------------------------------------------------+
  | value  | meaning (BlNackCode)                                        |
  +--------+-------------------------------------------------------------+
  | 0x00   | BL_NACK_OK              accepted / acted on                 |
  | 0x01   | BL_NACK_CRC            datagram_crc32 mismatch → resend      |
  | 0x02   | BL_NACK_SEQUENCE      out-of-order datagram_id → resend id   |
  | 0x03   | BL_NACK_INCOMPLETE    missing fragments → resend whole dg    |
  | 0x04   | BL_NACK_FLASH         erase/write/verify failed → abort      |
  | 0x05   | BL_NACK_OVERSIZED     image > app region → abort             |
  | 0x06   | BL_NACK_BAD_STATE     type illegal for current FSM state     |
  | 0x07   | BL_NACK_NO_APP        jump refused; app CRC/app_present bad   |
  | 0x08   | BL_NACK_INTERNAL      unspecified node-side failure          |
  +--------+-------------------------------------------------------------+
```

```c
typedef enum BlNackCode {
  BL_NACK_OK         = 0x00,
  BL_NACK_CRC        = 0x01,
  BL_NACK_SEQUENCE   = 0x02,
  BL_NACK_INCOMPLETE = 0x03,
  BL_NACK_FLASH      = 0x04,
  BL_NACK_OVERSIZED  = 0x05,
  BL_NACK_BAD_STATE  = 0x06,
  BL_NACK_NO_APP     = 0x07,
  BL_NACK_INTERNAL   = 0x08,
} BlNackCode;
```

**Retransmit policy (client side):** `CRC` / `SEQUENCE` / `INCOMPLETE` are *transient* —
the client retries the same `datagram_id` (narrowing the mask to laggards on a
multi-target flash, doc 07 §2). `FLASH` / `OVERSIZED` / `NO_APP` / `BAD_STATE` /
`INTERNAL` are *fatal* for that node — abort and report. The mapping from `BlStatus`
to `BlNackCode` is a small table in `bl_dfu.c` / the router.

---

## 3. Two layers: the superloop vs. the DFU session FSM

This is the part to get right conceptually. There are **two distinct layers**, and only
the second is a state machine. This mirrors FOTA exactly (`fota.c` superloop +
`FotaDFUState` inside `fota_dfu`):

### 3a. Layer 1 — the superloop (NOT a state machine)

The main loop does what FOTA's `fota_process()` does: **poll, poll, poll.** Reassembly
and CRC-verification of fragments is owned by the **transport** (doc 06/07) — the
superloop just pumps it. When a datagram completes, the transport fires `on_datagram`.

```c
for (;;) {
  if (can_enabled)  { bl_transport_can.poll();  }   /* reassembles frames → fires on_datagram() */
  if (uart_enabled) { bl_transport_uart.poll(); }
  if (bl_idle_timed_out() && !bl_dfu_active() && bl_app_valid()) {
    bl_jump_to_app();                                /* 5 s window expired, app is good */
  }
  bl_port_watchdog_kick();
}
```

The superloop has no states — it has the **5 s idle-timeout** and the watchdog kick,
nothing more. Fragment-level reception lives *below* it (in the transport), DFU
sequencing lives *above* it (in §3b). It never inspects a fragment or a datagram type.

### 3b. Layer 2 — the DFU session FSM (`bootloader/bl_dfu.c`)

`on_datagram(dg)` hands a **complete, CRC-verified** datagram to `bl_dfu_process(dg)`,
which steps a small **session** FSM keyed on the datagram's `type`. State names mirror
`FotaDFUState`. "RECEIVING" does **not** mean "receiving fragments" (that's Layer 1) — it
means **a DFU session is in progress**: `METADATA` has been accepted and we're
accumulating whole `CHUNK` datagrams.

```
   reset
     │
     ▼
 UNINITIALIZED ──bl_dfu_init() ok──▶ IDLE ◀──────────────────────────┐
                                     │ │ │                            │ chunk written + ACKed,
       QUERY / PING (answer, stay) ──┘ │ │                            │ bytes_written < binary_size
                                       │ │  valid METADATA            │
                  JUMP_APP / 5 s idle ─┘ └────────────▶ RECEIVING ────┘
                            │                              │  each CHUNK datagram:
                            ▼                              │  in-seq? → lazy-erase + write + ACK
                          JUMP                             │
            app CRC == config → bl_jump(app)  (MSP/VTOR)   │ bytes_written == binary_size
            app bad / app_present==0 → back to IDLE        ▼
                                                        COMPLETE
                                  pad final write to 8B (0xFF); CRC32 over binary_size:
                                    match    → write app_* to BootConfig → IDLE (app now valid)
                                    mismatch → NACK(BL_NACK_CRC), app_present=0 → IDLE (no brick)
```

- **`IDLE` → `RECEIVING`** happens the instant a valid `METADATA` datagram arrives. That's
  the "when do we jump into RECEIVING" answer: on `METADATA`, not on the first fragment.
- Entering `RECEIVING` **disarms the 5 s auto-jump** (`bl_dfu_active()` returns true), so a
  flash in progress is never interrupted by the idle timeout.
- `COMPLETE` and `JUMP` are **transient** — they act and return to a resting state; the
  superloop only ever sits in `IDLE` or `RECEIVING`.

### State table

| State | Entered when | Accepts (datagram type) | Notes |
|-------|--------------|-------------------------|-------|
| `BL_DFU_UNINITIALIZED` | before `bl_dfu_init()` | — | pre-init only |
| `BL_DFU_IDLE` | init done / session ended | `METADATA`→RECEIVING, `QUERY`/`PING` (answer, stay), `JUMP_APP`→JUMP | 5 s auto-jump armed here |
| `BL_DFU_RECEIVING` | valid `METADATA` | `CHUNK` (in-seq), `JUMP_APP` (abort session) | auto-jump disarmed; out-of-seq id → `BL_NACK_SEQUENCE` |
| `BL_DFU_COMPLETE` | `bytes_written == binary_size` | (internal) | CRC over `binary_size`; writes BootConfig on match; → IDLE |
| `BL_DFU_JUMP` | `JUMP_APP` or 5 s idle | (internal) | jump iff app CRC re-checks vs config; else → IDLE |

**Faults are not a state.** `bl_dfu_process` returns a `BlStatus`; the router maps it to a
`BlNackCode` and replies. A *transient* code (CRC/SEQUENCE/INCOMPLETE) keeps the session;
a *fatal* code sends NACK and (for genuinely unrecoverable cases) calls `bl_port_reset()`.
No perpetual-reset on unknown traffic, because the transport already filtered/bounds-checked
the role ID before any datagram reached this FSM (retires doc 02 §B2).

Key differences from the old FSM (doc 01 §4) and the doc-02 bugs they retire:
- Driven by **datagram `type`** handed up whole, not `arbitration_id_handle[msg->id]` on
  every raw frame → no out-of-bounds dispatch, no reset-on-every-unknown-ID (doc 02 §B2/§B3).
- A failed flash lands in `IDLE` with `app_present=0` — **never bricks**, just refuses to jump.
- `QUERY`/`PING` answered straight from `IDLE` without leaving it (discovery, doc 07 §6).

---

## 4. Clarifications resolved here (defaults adopted)

These were "left to implementation" (doc 07 §10) but touch P0–P2, so they're decided now.
Each can be revisited, but the build proceeds on these.

### 4a. x86 test geometry — fake-flash harness, not linker symbols

The core reads geometry from linker symbols (`_app_start`, `_config_start`,
`_flash_page_size`, …). x86 unit tests have no ARM linker, so the **`port/x86/` stub
owns geometry**: it defines a `static uint8_t s_fake_flash[BL_FLASH_SIZE]` byte array and
*provides the symbols* (`_app_start` etc. as addresses into that array). The same
`bootloader_user_config.h` feeds both — the test build `#include`s it to size the array,
so x86 and ARM can't disagree (kills the doc-02 §A4 "three copies" risk). `bl_port_flash_*`
on x86 just `memcpy`/`memset` within `s_fake_flash`. **No second source of geometry.**

### 4b. No-init RAM magic word — in the static section template

The app→BL forced-entry word (doc 07 §7) is a fixed, invariant reservation, so it lives in
the **static `sections_common.ld`**, *not* the generated `memory_map.ld`:

```ld
.noinit (NOLOAD) : {
  . = ALIGN(4);
  *(.bl_noinit)          /* BlEntryMagic — survives reset, not zeroed at startup */
  . = ALIGN(4);
} > RAM
```

A single `uint32_t __attribute__((section(".bl_noinit")))` holds `BL_ENTER_MAGIC`. The
generator stays purely about flash geometry; the RAM word never changes per chip.

### 4c. CRC-32 is streaming (init / update / finalize)

The post-flash whole-image check CRCs ~421 KB read back from flash, which can't buffer in
RAM. So `bootloader_crc32` exposes an incremental API (one-shot is a thin wrapper):

```c
void     bl_crc32_init(BlCrc32 *ctx);
void     bl_crc32_update(BlCrc32 *ctx, const uint8_t *buf, uint32_t len);
uint32_t bl_crc32_finalize(BlCrc32 *ctx);
uint32_t bl_crc32(const uint8_t *buf, uint32_t len);   /* convenience one-shot */
```

Algorithm is the single spec'd zlib-reflected CRC-32 (poly `0xEDB88320`, init `0xFFFFFFFF`,
final XOR `0xFFFFFFFF`) so a streamed result == `zlib.crc32` over the same bytes (doc 04 §3).

### 4d. Error type is `BlStatus`, NACK byte is `BlNackCode`

The core uses its own `BlStatus` (§1) — **not** the repo `StatusCode`, **not** `FotaError`
— preserving zero host-repo coupling. The wire reports the coarse `BlNackCode` (§2).
```
