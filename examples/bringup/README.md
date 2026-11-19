# Genesis Rev 1 bringup example

Smallest useful firmware that exercises this BSP: it runs the mandated
`IO Expander → PMU → SH8601` sequence, asserts the 30 % brightness gate, prints
PMU battery state and reports FT3168 touch coordinates.

## Build and run

From the BSP root:

```bash
cp sdkconfig.defaults examples/bringup/
cp partitions.csv examples/bringup/
cd examples/bringup
source /path/to/esp-idf/export.sh       # ESP-IDF 6.1 or newer
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

Alternatively set `SDKCONFIG_DEFAULTS` to the BSP root file when configuring:

```bash
export SDKCONFIG_DEFAULTS="$PWD/sdkconfig.defaults"
```

Expected successful boot lines:

```
I genesis_bringup: CsAW Genesis Rev 1 bringup example
I csaw_hal: STAGE 1 OK: IO expander 0x20, pins 0-2 rails enabled
I csaw_hal: STAGE 2 OK: AXP2101 PMU rails verified
I csaw_hal: STAGE 3 OK: SH8601 AMOLED ready (368x448)
I genesis_bringup: Bringup OK: expander -> PMU -> SH8601, brightness >= 30%
```

If you instead see `IO expander not found at 0x20: ESP_ERR_TIMEOUT`, the board
is not powered or is not a Rev 1 SH8601/FT3168 board. See
[docs/VERIFIED_BUILD.md](../../docs/VERIFIED_BUILD.md) — this exact failure was
observed on the first hardware attempt.

## Status

This example was build-verified with ESP-IDF 6.1 (`Project build complete`,
`genesis_bringup.bin` `0x54c00`; log `/tmp/opencode/bsp_build_check.log`) but has
**not been confirmed running on a working board**; the first hardware attempt
failed before the I²C expander responded.
