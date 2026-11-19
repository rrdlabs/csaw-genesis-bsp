# CsAW Genesis BSP Changelog

Every change bumps [VERSION.md](VERSION.md) and appends an entry with date,
version, summary, files touched and verification result.

## [0.1.2] - 2026-11-18

### Summary

Published the BSP to GitHub as a public repository (matching the visibility of
the main firmware repository) and recorded the exact creation/push command and
its result. The published state carries the explicit hardware FAIL caveat.

### Files touched

- `CHANGELOG.md`, `VERSION.md`, `docs/VERIFIED_BUILD.md` — publication evidence and version increment.

### Verification result

- `gh repo create rrdlabs/csaw-genesis-bsp --public --source=. --remote=origin --push` → exit 0, created `https://github.com/rrdlabs/csaw-genesis-bsp`, `* [new branch] HEAD -> master`.
- Initial commit `69b08c6`; `git remote -v` → `origin https://github.com/rrdlabs/csaw-genesis-bsp.git` (fetch/push).
- Branding zero-hit check re-run before publication → zero hits. Hardware status unchanged: **FAIL**.

## [0.1.1] - 2026-11-18

### Summary

Made the minimal bringup example self-contained: ESP-IDF could not find the
`csaw_hal` component because the BSP components live two levels above the
example. Added `EXTRA_COMPONENT_DIRS` to the example project, then built the
example successfully with the same ESP-IDF 6.1 toolchain used for the firmware.
Also removed literal donor-brand strings from the verification prose so the
repository is a true zero-hit for unapproved branding, and confirmed the
corrected esptool v5 option order in `scripts/flash.sh` with `bash -n`.

### Files touched

- `examples/bringup/CMakeLists.txt` — added `EXTRA_COMPONENT_DIRS` pointing at the BSP root `components/`.
- `examples/bringup/README.md`, `docs/VERIFIED_BUILD.md`, `CHANGELOG.md`, `VERSION.md` — recorded the new build evidence and hardware status.
- `scripts/flash.sh` — marked executable; syntax validated (`bash -n`).

### Verification result

- `bash -n scripts/flash.sh` → exit 0; mode `755`.
- Case-insensitive recursive search for the donor-brand string (excluding `.git`) → zero hits.
- BSP example build: copied the BSP to `/tmp/opencode/bsp_build_check`, copied `sdkconfig.defaults`/`partitions.csv` into `examples/bringup`, ran `idf.py set-target esp32s3 && idf.py build` → exit 0, `Project build complete`, `genesis_bringup.bin` `0x54c00` (83 % free in 2 MB slot). Full log `/tmp/opencode/bsp_build_check.log`.
- Hardware status unchanged: **FAIL** (see 0.1.0 and `docs/VERIFIED_BUILD.md`).

## [0.1.0] - 2026-11-18

### Summary

Initial board-support package for the Waveshare ESP32-S3-TOUCH-AMOLED-1.8 Rev 1
("Genesis"). Extracted from the build-verified `csaw_esp32s3_genesis` firmware
v1.1.1 so downstream projects can add this exact board without pulling in the
CsAW agent, UI, storage, tools or MicroPython VM. Hardware verification is
explicitly marked **FAILED**: a device was flashed successfully but no I²C
peripherals responded and the device disconnected before diagnosis. The operator
chose to publish the BSP with this status rather than wait.

### Files touched

- `components/csaw_hal/**` — native ESP-IDF 6.1 HAL (literal expander → PMU → SH8601 bringup, 30 % brightness gate, touch/audio/RTC/SDMMC/IMU, PMU battery/backup charging, RTC alarms/timers, IMU wake-on-motion). Copied verbatim from firmware.
- `components/cjson/**` — cJSON v1.7.19 compatibility component (IDF 6.1 removed the `json` component). Optional for downstream projects.
- `config/hardware.md`, `docs/HARDWARE_BRINGUP.md`, `docs/DISPLAY_BRINGUP.md`, `docs/HARDWARE_SOURCES.md` — board spec, register table and cited source links. Copied verbatim from firmware.
- `partitions.csv` — exact verified partition table (2 MB ota_0/ota_1, 12 MB SPIFFS, coredump).
- `sdkconfig.defaults` — IDF 6.1 target/flash/PSRAM/partition/console settings reconciled to the verified image (DIO 80 MHz 16 MB).
- `examples/bringup/**` — minimal buildable firmware demonstrating the mandated init order and brightness assertion.
- `docs/PIN_MAP.md` (new), `docs/VERIFIED_BUILD.md` (new) — consolidated pin/bus/delay/register reference and the exact build/merge verification evidence plus hardware blocker.
- `scripts/flash.sh` (new) — merged-image flashing helper.
- `README.md`, `VERSION.md`, `CHANGELOG.md`, `LICENSE` — package metadata; license retains upstream attribution as legally required.

### Verification result

- Provenance: all copied files are byte-identical to their firmware sources — verified with `sha256sum` comparison at extraction time.
- Toolchain: firmware compiled with ESP-IDF v6.1 at `/home/d31337m3/devenv/esp-idf-6.1`; final firmware build log `/tmp/opencode/csaw_wp7_build23.log` → `Project build complete`.
- Merged image: `/tmp/opencode/csaw_merge_bin.log` → exit 0; `merged-binary.bin` SHA-256 `bc99701b6d3d819de63e436391d5809129d5158af05313dd27dc6f4abc66d2ee`; all five embedded regions byte-identical to source binaries.
- Branding: case-insensitive recursive search for the donor-brand string (excluding `.git`) → zero hits.
- **Hardware: FAIL.** A device at `/dev/ttyACM0` accepted the merged image and `Hash of data verified` passed, but boot logged `IO expander not found at 0x20: ESP_ERR_TIMEOUT`, `Hardware bringup FAILED`, `SD card mount /sd unavailable` and `On-device UI init FAILED`. A standalone scan found zero I²C devices on SDA15/SCL14 (400 kHz and 100 kHz) or swapped pins. The device then disconnected. The operator chose to publish with this failure clearly labelled.
