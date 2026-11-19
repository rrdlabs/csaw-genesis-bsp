# Build verification evidence and hardware gate

This file is the audit trail for the claim "derived from a build-verified
firmware". It intentionally separates **what was verified** from **what was
not**.

## Verified: firmware source and merged image

Parent repository: `rrdlabs/csaw_esp32s3_genesis` @ `1.1.0` (commit `78f006f`).

| Check | Evidence | Result |
| --- | --- | --- |
| Toolchain | `/home/d31337m3/devenv/env-6.1.sh` | `ESP-IDF v6.1` (Warroom-provisioned) |
| Firmware build | `/tmp/opencode/csaw_wp7_build23.log` — `cd …/csaw_esp32s3_genesis && bash -c 'source /home/d31337m3/devenv/env-6.1.sh >/dev/null 2>&1; idf.py build'` | exit 0, `Project build complete`, app `0x1ecad0`, 4 % free in 2 MB slot |
| BSP example build | `/tmp/opencode/bsp_build_check.log` — temp copy of this BSP, `cd examples/bringup && cp ../../sdkconfig.defaults ../../partitions.csv . && idf.py set-target esp32s3 && idf.py build` | exit 0, `Project build complete`, `genesis_bringup.bin` `0x54c00`, 83 % free in 2 MB slot |
| Merged image | `/tmp/opencode/csaw_merge_bin.log` — same env + `idf.py merge-bin` | exit 0, `Successfully created ESP32-S3 image`, `Wrote 0xff0000 bytes` |
| Image digest | `sha256sum build/merged-binary.bin` | `bc99701b6d3d819de63e436391d5809129d5158af05313dd27dc6f4abc66d2ee` |
| Embedded regions | Python byte comparison of each component against its slice of the merged image | `MERGED_IMAGE_INTEGRITY = PASS`; all gaps `0xFF` |
| Headers | `esptool image-info bootloader/bootloader.bin`, `… csaw.bin` | both Chip ID 9 / 16 MB / DIO / 80 m; checksums `0x86` and `0x08` valid; ESP-IDF v6.1 |
| Partition table | `gen_esp32part.py partition-table.bin` | `Verifying table...` OK |
| Brand scan | case-insensitive recursive search for the donor-brand string (excluding `.git`) | zero hits |

The merged image `build/merged-binary.bin` is a local IDF artifact (git-ignored),
not stored in git. Regenerate it in the parent firmware repo with:

```bash
source /home/d31337m3/devenv/env-6.1.sh
idf.py build && idf.py merge-bin
```

## Publication

```bash
$ gh repo create rrdlabs/csaw-genesis-bsp --public --source=. --remote=origin --push
https://github.com/rrdlabs/csaw-genesis-bsp
To https://github.com/rrdlabs/csaw-genesis-bsp.git
 * [new branch]      HEAD -> master
```
(exit 0; initial commit `69b08c6`, branch `master`, public — matching the main
firmware repository's visibility.)

## Physical hardware: attempted and FAILED

A device appeared briefly at `/dev/ttyACM0` (Espressif USB JTAG/serial debug
unit, USB `303a:1001`, MAC `80:b5:4e:db:3c:90`).

**Flash — PASS** (`/tmp/opencode/csaw_flash_genesis.log`):

```
$ python -m esptool --chip esp32s3 --port /dev/ttyACM0 -b 460800 \
    --before default-reset --after hard-reset write-flash \
    --flash-mode dio --flash-size 16MB --flash-freq 80m \
    0x0 build/merged-binary.bin
Wrote 16711680 bytes (1431091 compressed) at 0x00000000 in 80.7 seconds.
Verifying written data...
Hash of data verified.
```
(Note: esptool v5 requires the flash options after `write-flash`; the earlier
order produced `No such option '--flash-mode'`. `scripts/flash.sh` is corrected.)

**Bringup — FAIL** (`/tmp/opencode/csaw_boot_monitor.log`):

```
I (372) csaw:   CsAW - ESP32-S3 AI Agent
I (372) csaw_hal: I2C0 ready (SDA=15 SCL=14, 400000 Hz)
E (572) i2c.master: probe device timeout. Please check if xfer_timeout_ms and pull-ups are correctly set up
E (572) csaw_hal: IO expander not found at 0x20: ESP_ERR_TIMEOUT
E (572) csaw: Hardware bringup FAILED: ESP_ERR_TIMEOUT
E (1152) storage: SD card mount /sd unavailable; high-churn storage disabled
E (1152) csaw: On-device UI init FAILED: ESP_ERR_INVALID_STATE
```

A standalone scan (`/tmp/opencode/csaw_probe_monitor.log`) probed `0x01..0x77`
on SDA15/SCL14 at 400 kHz and 100 kHz and on swapped pins 14/15: **zero
devices responded**, including the AXP2101 PMU at `0x34` and RTC at `0x51`.
The vendor reference (`examples/esp-idf/90_axp2101_pmu/components/board_variant/
board_variant.c`) confirms `0x20`, SDA=15/SCL=14 and 400 kHz are correct for
Rev 1, so this is not a wrong address or pin in the HAL. The device then
disconnected before a GPIO-level diagnostic could run, so the root cause is
unresolved — consistent with the attached device being unpowered, not a Rev 1
board, or an emulation without the Waveshare peripherals.

Therefore this BSP does **not** claim that the panel lights, touch responds, the
SD card mounts, or any peripheral probes successfully on silicon. The operator
elected to publish with this failure explicitly recorded.

## Hardware acceptance checklist (run on a genuine powered Rev 1 board)

1. Confirm the board is a Rev 1 SH8601 + FT3168 board and is powered.
2. `scripts/flash.sh /dev/ttyUSB0 /path/to/merged-binary.bin`
3. `idf.py -p /dev/ttyUSB0 monitor`
4. Confirm log: `STAGE 1 OK: IO expander 0x20`, `STAGE 2 OK: AXP2101`,
   `STAGE 3 OK: SH8601`, and `brightness >= 30% asserted before UI`.
5. Confirm non-fatal peripheral lines: touch, audio, RTC, `SDMMC ready at /sd`,
   IMU.
6. Visually confirm the AMOLED panel is lit and the on-screen UI renders.
7. Touch the panel and confirm the FT3168 INT wakes/reports a coordinate.

Once all seven pass, update this file, the README status banner and
[CHANGELOG.md](../CHANGELOG.md) with the board serial/date and bump the BSP
version to `0.2.0`.
