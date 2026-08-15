// Die ISS-Box — Stage 3: temperature and humidity
//
// A first cut of WETTER mode. The LCD shows the numbers, the ring becomes a
// thermometer arc growing blue-to-red with temperature, and the digits show
// the reading — or as much of it as the digits currently fitted allow.

#include <cstdio>

#include "board_config.hpp"
#include "drivers/i2c_bus.hpp"
#include "drivers/lcd1602_rgb.hpp"
#include "drivers/neopixels.hpp"
#include "drivers/seven_segment.hpp"
#include "drivers/sht_sensor.hpp"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "tools/i2c_scan.hpp"

namespace {

constexpr uint  kFrameMs = 40;
constexpr float kTempMinC = 15.0f;  // ring empty at or below this
constexpr float kTempMaxC = 30.0f;  // ring full at or above this

bool led_init() { return cyw43_arch_init() == 0; }

void wait_for_console(uint32_t timeout_ms = 3000) {
    absolute_time_t deadline = make_timeout_time_ms(timeout_ms);
    while (!stdio_usb_connected() && !time_reached(deadline)) {
        sleep_ms(50);
    }
}

// Thermometer arc: a blue-to-red gradient whose length tracks temperature.
void draw_thermometer(drivers::NeoPixels &ring, float temp_c) {
    float frac = (temp_c - kTempMinC) / (kTempMaxC - kTempMinC);
    if (frac < 0.0f) frac = 0.0f;
    if (frac > 1.0f) frac = 1.0f;

    const uint lit = static_cast<uint>(frac * static_cast<float>(ring.count()) + 0.5f);

    ring.clear();
    for (uint i = 0; i < lit; i++) {
        // Hue by position rather than by temperature, so the arc reads as a
        // scale that fills up rather than a bar that changes colour.
        const float hue = 240.0f * (1.0f - static_cast<float>(i) /
                                               static_cast<float>(ring.count() - 1));
        ring.set_hsv(i, hue, 1.0f, 0.6f);
    }
}

// Shows as much of the temperature as the fitted digits allow: the full
// "23.4" once four are wired, otherwise just the tens.
void show_temperature(drivers::SevenSegment &digits, float temp_c, uint fitted) {
    if (fitted >= 4) {
        const int scaled = static_cast<int>(temp_c * 10.0f + 0.5f);
        digits.show_number(scaled);
        digits.set_dot(2, true);  // between the units and the tenths
    } else {
        const int whole = static_cast<int>(temp_c + 0.5f);
        digits.set_digit(0, (whole / 10) % 10);
    }
}

}  // namespace

int main() {
    stdio_init_all();
    wait_for_console();
    led_init();

    printf("\n=== ISS-Box Stage 3: temperature + humidity ===\n");

    drivers::i2c_bus().init();
    tools::i2c_scan();

    drivers::Lcd1602Rgb lcd(drivers::i2c_bus());
    const bool          lcd_ok = lcd.init();

    drivers::ShtSensor sht(drivers::i2c_bus());
    const bool         sht_ok = sht.init();
    if (sht_ok) {
        printf("sensor: %s, id 0x%08lX\n", sht.kind_name(), (unsigned long)sht.id());
    } else {
        printf("no SHT sensor answered at 0x%02X\n", drivers::ShtSensor::kDefaultAddr);
        if (lcd_ok) {
            lcd.set_backlight(120, 0, 0);
            lcd.print_line(0, "Kein Sensor!");
            lcd.print_line(1, "0x44 stumm");
        }
    }

    drivers::NeoPixels ring(board::PIN_RING, board::RING_PIXELS, 300);
    ring.init();

    drivers::NeoPixels digit_pixels(board::PIN_DIGITS, board::DIGIT_PIXELS_FITTED, 300);
    digit_pixels.init();
    digit_pixels.set_master_brightness(120);
    drivers::SevenSegment digits(digit_pixels, board::DIGITS_FITTED);

    uint32_t frame = 0;
    uint32_t bad_reads = 0;

    while (true) {
        // The sensor is far slower than the display, and self-heats if polled
        // hard, so once a second is plenty.
        if (frame % (1000 / kFrameMs) == 0 && sht_ok) {
            if (sht.read()) {
                const float t = sht.temperature_c();
                const float h = sht.humidity_pct();

                printf("%.2f C   %.1f %%RH   (%lu failed reads)\n", (double)t, (double)h,
                       (unsigned long)bad_reads);

                if (lcd_ok) {
                    char line[17];
                    snprintf(line, sizeof(line), "%5.1f C", (double)t);
                    lcd.print_line(0, line);
                    snprintf(line, sizeof(line), "%5.1f %%rF", (double)h);
                    lcd.print_line(1, line);
                    // Backlight warms with temperature, same scale as the ring.
                    float frac = (t - kTempMinC) / (kTempMaxC - kTempMinC);
                    if (frac < 0.0f) frac = 0.0f;
                    if (frac > 1.0f) frac = 1.0f;
                    lcd.set_backlight_hue(240.0f * (1.0f - frac));
                }

                draw_thermometer(ring, t);
                ring.show();

                digits.clear();
                show_temperature(digits, t, board::DIGITS_FITTED);
                digits.show();
            } else {
                bad_reads++;
                printf("read failed (CRC or bus), total %lu\n", (unsigned long)bad_reads);
            }
        }

        frame++;
        sleep_ms(kFrameMs);
    }
}
