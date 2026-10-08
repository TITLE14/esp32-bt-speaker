#include <stdint.h>
#include "freertos/idf_additions.h"
#include "power_monitor.h"
#include "display.h"

typedef struct {
    float voltage;
    uint8_t level;
} voltage_margin_point;

static const voltage_margin_point inr18650_lut[] = {
    {4.20f, 100},
    {4.12f,  95},
    {4.07f,  90},
    {4.00f,  85},
    {3.95f,  80},

    {3.85f,  70},
    {3.75f,  60},
    {3.70f,  50},
    {3.65f,  42},
    {3.60f,  34},

    {3.50f,  24},
    {3.40f,  15},
    {3.35f,  10},
    {3.30f,   7},
    {3.25f,   4},
    {3.15f,   0},
};

static uint8_t get_battery_level(void) {
    pwr_data pwr = pw_get_data();
    float voltage = pwr.voltage;
    uint8_t lut_size = sizeof(inr18650_lut) / sizeof(voltage_margin_point);

    if(voltage >= inr18650_lut[0].voltage) {
        return inr18650_lut[0].level;
    }
    if(voltage <= inr18650_lut[lut_size - 1].voltage) {
        return inr18650_lut[lut_size - 1].level;
    }

    for(int i = 1; i < sizeof(inr18650_lut)/sizeof(voltage_margin_point); ++i) {
        if(voltage >= inr18650_lut[i].voltage) {
            voltage_margin_point lower_p = inr18650_lut[i];
            voltage_margin_point upper_p = inr18650_lut[i - 1];

            float battery_level = lower_p.level + ((voltage - lower_p.voltage) * (upper_p.level - lower_p.level)) / (upper_p.voltage - lower_p.voltage);
            return battery_level + 0.5f;
        }
    }
    return 0;
}

static void battery_task(void* param) {
    while(1) {
        static uint8_t level = 0;
        uint8_t new_level = get_battery_level();

        if (level != new_level) {
            display_draw_battery_level(new_level);
            display_update(BATTERY_AREA);
            level = new_level;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void battery_init(void) {
    pw_init();
    xTaskCreate(battery_task, "Battery_task", 4096, NULL, 3, NULL);
}