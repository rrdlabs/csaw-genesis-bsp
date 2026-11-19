/*
 * Minimal Genesis Rev 1 bringup example.
 *
 * Demonstrates the mandated sequence (IO expander -> PMU -> SH8601) and the
 * >=30% brightness assertion without any CsAW application code. See
 * docs/PIN_MAP.md and docs/HARDWARE_BRINGUP.md in the BSP root.
 */
#include <assert.h>
#include <stdio.h>

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "csaw_hal.h"

static const char *TAG = "genesis_bringup";

void app_main(void)
{
    bool brightness_asserted = false;

    ESP_LOGI(TAG, "CsAW Genesis Rev 1 bringup example");
    esp_err_t err = csaw_hal_init(&brightness_asserted);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Bringup FAILED: %s", esp_err_to_name(err));
        return;
    }
    if (!brightness_asserted) {
        ESP_LOGE(TAG, "Bringup FAILED: brightness gate not asserted");
        return;
    }
    ESP_LOGI(TAG, "Bringup OK: expander -> PMU -> SH8601, brightness >= 30%%");

    uint8_t level = 0;
    if (csaw_hal_display_brightness_get(&level) == ESP_OK) {
        ESP_LOGI(TAG, "Display brightness = %u%%", level);
    }

    csaw_hal_battery_status_t battery;
    if (csaw_hal_pmu_battery_status(&battery) == ESP_OK) {
        ESP_LOGI(TAG, "Battery: %u%%, %u mV, charging=%d, present=%d",
                 battery.percent, battery.voltage_mv,
                 battery.charging, battery.battery_present);
    }

    ESP_LOGI(TAG, "Touch test: press the panel (INT=GPIO21)");
    while (true) {
        uint16_t x = 0, y = 0;
        bool pressed = false;
        if (csaw_hal_touch_read(&x, &y, &pressed) == ESP_OK && pressed) {
            ESP_LOGI(TAG, "Touch: x=%u y=%u", x, y);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
