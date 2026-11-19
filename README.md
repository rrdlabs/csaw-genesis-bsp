# CsAW Genesis BSP

**Board support package for the Waveshare ESP32-S3-TOUCH-AMOLED-1.8, Rev 1 —
"Genesis"** (SH8601 AMOLED + FT3168 touch, original panel revision).

> [!WARNING]
> **Hardware verification FAILED on the first test — do not treat this BSP as
> hardware-certified.** This BSP is derived from the build-verified
> `csaw_esp32s3_genesis` firmware: the native ESP-IDF 6.1 HAL, pin map, register
> sequence and build configuration compile cleanly, and the merged flash image
> passes byte-level verification. A device briefly appeared at `/dev/ttyACM0`;
> writing the image and verifying the hash **passed**, but the flashed firmware
> reported `IO expander not found at 0x20: ESP_ERR_TIMEOUT`, an independent I²C
> scan found **zero** devices, and SD/UI initialization failed. The device then
> disconnected. The operator chose to publish the BSP anyway with this failure
> clearly recorded. See [docs/VERIFIED_BUILD.md](docs/VERIFIED_BUILD.md).

## Scope

This repository contains **board support only** so that developers can add this
exact board to their own ESP-IDF projects. It deliberately does **not** contain
the CsAW agent, LLM providers, chat UI, storage policy, cron, or MicroPython VM.
Downstream projects bring their own application; they consume the HAL and the
board configuration from here.

| Included | Not included |
| --- | --- |
| Native ESP-IDF 6.1 HAL for all eight peripherals | Agent loop / LLM providers |
| Literal IO expander → PMU → SH8601 bringup + 30 % brightness gate | LVGL chat UI / keyboard |
| Pin map, QSPI register table, reset delays | Session/context storage policy |
| `sdkconfig.defaults`, `partitions.csv` | SD filesystem tools / cron |
| Minimal `examples/bringup` firmware | MicroPython VM |

## Quick start (downstream project)

```
your-project/
├── CMakeLists.txt
├── sdkconfig.defaults      <- copy from this BSP, or include its settings
├── partitions.csv          <- copy from this BSP
└── components/
    ├── csaw_hal/           <- copy from this BSP
    └── cjson/              <- optional: only if IDF < 6 removed `json`
```

Minimal root `CMakeLists.txt` (ESP-IDF 6.1 standard):

```cmake
cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(my_genesis_app)
```

Minimal `main/main.c`:

```c
#include "csaw_hal.h"

void app_main(void) {
    bool brightness_asserted = false;
    ESP_ERROR_CHECK(csaw_hal_init(&brightness_asserted));
    assert(brightness_asserted);   // >=30% set before any UI is drawn
}
```

Then:

```bash
source /path/to/esp-idf/export.sh      # must be ESP-IDF 6.1 or newer
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

A complete, buildable example lives in [`examples/bringup`](examples/bringup).
Run `scripts/flash.sh /dev/ttyUSB0 build/merged-binary.bin` if you have a merged
image from the parent firmware repo.

## Board facts (Rev 1 only)

| Peripheral | Detail |
| --- | --- |
| Display | SH8601, 368×448 QSPI AMOLED, RGB565, SPI2_HOST |
| QSPI pins | PCLK=11, CS=12, DATA0–3 = 4,5,6,7 |
| Touch | FT3168 (FT5x06-compatible), I²C, INT=21 |
| I²C | SCL=14, SDA=15 |
| IO expander | TCA9554/XCA9554 @ `0x20` (enable rails before PMU) |
| PMU | AXP2101 @ `0x34` (battery + RTC backup-cell charging) |
| RTC | PCF85063A @ `0x51` |
| Audio | ES8311 @ `0x18`; I2S MCLK=16, BCLK=9, WS=45, DO=10, DI=8, PA=46 |
| SD card | SDMMC 1-bit: CLK=2, CMD=1, D0=3; mounted at `/sd` |
| IMU | QMI8658 @ `0x6B` (fallback `0x6A`) |
| Brightness floor | 30 % (register `0x51` ≥ `0x4D`) immediately after AMOLED init |
| Flash / PSRAM | 16 MB flash (verified image DIO @ 80 MHz), 8 MB octal PSRAM @ 80 MHz |

Full detail: [config/hardware.md](config/hardware.md),
[docs/PIN_MAP.md](docs/PIN_MAP.md),
[docs/HARDWARE_BRINGUP.md](docs/HARDWARE_BRINGUP.md),
[docs/DISPLAY_BRINGUP.md](docs/DISPLAY_BRINGUP.md).

**Do not use this BSP for the V2 board** (CO5300 display / CST820 touch) — the
register table and pin map differ. Rev 2 is out of scope.

## Provenance

Derived from the `csaw_esp32s3_genesis` firmware repository at version `1.1.0`.
Hardware decisions cite the Waveshare/Espressif sources recorded in
[docs/HARDWARE_SOURCES.md](docs/HARDWARE_SOURCES.md); no pin, delay or register
value was guessed. The `LICENSE` file retains the upstream copyright holder
as legally required attribution; it is not product branding.

## Status / version

- BSP version: `0.1.0` (see [VERSION.md](VERSION.md) / [CHANGELOG.md](CHANGELOG.md))
- Toolchain verified: ESP-IDF v6.1 (Warroom-provisioned)
- Firmware build verified: yes — `Project build complete`, app `0x1ecad0`
- Image flash verified: yes — `Hash of data verified` on `/dev/ttyACM0`
- Hardware bringup on that device: **FAILED** — no responding I²C peripherals;
  root cause unresolved because the board disconnected
