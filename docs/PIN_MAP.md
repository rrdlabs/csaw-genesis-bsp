# Genesis Rev 1 — consolidated pin, bus, delay and register map

Scope: **Waveshare ESP32-S3-TOUCH-AMOLED-1.8 Revision 1** (original SH8601 +
FT3168). Do not apply to Rev 2 (CO5300 + CST820). Every value below is taken
from the Waveshare/Espressif references in
[HARDWARE_SOURCES.md](HARDWARE_SOURCES.md) and is implemented in
`components/csaw_hal/csaw_hal.c`. The literal line-by-line bringup derivation is
in [DISPLAY_BRINGUP.md](DISPLAY_BRINGUP.md).

## 1. Mandated init order

```
IO Expander (TCA9554/XCA9554 0x20)
  -> PMU (AXP2101 0x34)
    -> SH8601 AMOLED (QSPI)
```

Each stage performs its own reset, register setup and readiness check before the
next begins. The sequence must not be collapsed or reordered. Immediately after
AMOLED init, brightness is set to **≥30 %** (`CSAW_LCD_BRIGHTNESS_MIN`) before
any UI is drawn. `csaw_hal_init()` returns this assertion via its
`brightness_asserted` out-parameter.

## 2. buses

| Bus | Setting | Pins |
| --- | --- | --- |
| I²C master | 400 kHz (vendor) | SCL **14**, SDA **15** |
| LCD QSPI | SPI2_HOST, DMA auto, 16 bpp RGB565 | PCLK **11**, CS **12**, D0 **4**, D1 **5**, D2 **6**, D3 **7** |
| I²S (audio) | ES8311, standard mode | MCLK **16**, BCLK **9**, WS **45**, DO **10**, DI **8**, PA **46** |
| SDMMC | 1-bit, 20 MHz, slot 1 | CLK **2**, CMD **1**, D0 **3** |
| Touch INT | FT3168, active low | GPIO **21** |

Panel geometry: 368 × 448, `LCD_RGB_ELEMENT_ORDER_RGB` (not BGR). Hardware RST
is **not connected** (`GPIO_NUM_NC`); the panel uses software reset `0x01`.

## 3. I²C device map

| Device | Address | Notes |
| --- | --- | --- |
| TCA9554/XCA9554 IO expander | `0x20` | Pins 0–2 LOW → 20 ms → HIGH enables rails |
| AXP2101 PMU | `0x34` | Rail verification, fuel gauge `0xA4`, backup-cell charging |
| PCF85063A RTC | `0x51` | BCD calendar `0x04..0x0A`, alarm + 1 Hz timer |
| ES8311 audio codec | `0x18` | I²S + PA on GPIO 46 |
| QMI8658 IMU | `0x6B` (fallback `0x6A`) | Wake-on-motion 128 Hz, 200 mg threshold |
| FT3168 touch | FT5x06-compatible | Registers `0x02..0x06` for single touch |

## 4. Brightness gate

`CSAW_LCD_BRIGHTNESS_MIN = 30`. The SH8601 brightness register is `0x51`;
30 % corresponds to ≥ `0x4D` in the 8-bit duty register. The HAL calls
`csaw_hal_display_brightness_set(30)` inside `csaw_hal_display_init()` (source
line `csaw_hal.c:490`), before `csaw_hal_init()` returns and therefore before
any application draws. This exists because the panel otherwise stays black and
appears unpowered.

## 5. Reset delays and register writes

The complete ordered register table, each delay and the vendor demo
line-by-line mapping are in [DISPLAY_BRINGUP.md](DISPLAY_BRINGUP.md). The PMU
rail sequence and non-display peripheral setup are in
[HARDWARE_BRINGUP.md](HARDWARE_BRINGUP.md). Do not substitute values from the
Rev 2 BSP (`waveshare/esp32_s3_touch_amoled_1_8` 2.0.3) for this board; the
Rev 1 reference is 1.1.4.

## 6. Verification status

These values are **build-verified** (the HAL compiles for ESP-IDF 6.1 and the
merged image is structurally valid) but **not hardware-verified**: no board was
attached when this BSP was created. After flashing, confirm the boot log
reports the expander → PMU → SH8601 sequence and the ≥30 % brightness gate.
