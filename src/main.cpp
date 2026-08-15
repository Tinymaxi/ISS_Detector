// Die ISS-Box — Stages 5 and 6: NeoPixel ring and seven-segment digits
//
// Two independent chains, one data pin each. A comet orbits the ring with a
// fading tail — deliberately previewing ISS mode, where a single bright pixel
// marks the station's bearing — while the digits count seconds with a blinking
// separator, previewing clock mode.

#include <cstdio>

#include "board_config.hpp"
#include "drivers/i2c_bus.hpp"
#include "drivers/lcd1602_rgb.hpp"
#include "drivers/neopixels.hpp"
#include "drivers/seven_segment.hpp"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "tools/i2c_scan.hpp"

namespace {

constexpr uint  kTailLength = 8;
constexpr uint  kFrameMs = 40;
constexpr float kHueStepPerFrame = 1.5f;
constexpr uint  kFramesPerSecond = 1000 / kFrameMs;

// On Pico W the onboard LED is driven by the wireless chip, not by a GPIO.
bool led_init() { return cyw43_arch_init() == 0; }
void led_set(bool on) { cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on); }

void wait_for_console(uint32_t timeout_ms = 3000) {
    absolute_time_t deadline = make_timeout_time_ms(timeout_ms);
    while (!stdio_usb_connected() && !time_reached(deadline)) {
        sleep_ms(50);
    }
}

}  // namespace

int main() {
    stdio_init_all();
    wait_for_console();

    if (!led_init()) {
        while (true) {
            printf("cyw43_arch_init() failed\n");
            sleep_ms(1000);
        }
    }

    printf("\n=== ISS-Box Stages 5+6 ===\n");
    printf("board: %s   sdk: %s\n", PICO_BOARD, PICO_SDK_VERSION_STRING);

    drivers::i2c_bus().init();
    tools::i2c_scan();

    drivers::Lcd1602Rgb lcd(drivers::i2c_bus());
    const bool          lcd_ok = lcd.init();
    lcd.set_backlight(0, 40, 80);
    lcd.print_line(0, "Ring + Ziffern");

    // Each chain clamps its own frame, so the budgets have to be chosen to sum
    // to something the supply can give: 300 + 300 mA plus ~50 mA for the Pico
    // is comfortable on laptop USB. Raise both once the box runs from the 5 V
    // Raspberry Pi supply.
    drivers::NeoPixels ring(board::PIN_RING, board::RING_PIXELS, 300);
    ring.init();

    drivers::NeoPixels digit_pixels(board::PIN_DIGITS, board::DIGIT_PIXELS, 300);
    digit_pixels.init();
    // Digits are held below the clamp deliberately. If the budget were doing
    // the limiting, "1111" would glow brighter than "8888" — brightness would
    // track how many segments happen to be lit, which looks like a fault.
    // At 120 even the worst case (8888 with every dot) lands near 140 mA.
    digit_pixels.set_master_brightness(120);
    drivers::SevenSegment digits(digit_pixels, board::DIGIT_COUNT);

    printf("ring: %u px on GP%u   digits: %u px on GP%u\n", ring.count(), board::PIN_RING,
           digit_pixels.count(), board::PIN_DIGITS);

    uint     head = 0;
    float    hue = 0.0f;
    uint32_t frame = 0;
    uint32_t seconds = 0;

    while (true) {
        // --- ring: comet with a fading tail ---
        ring.clear();
        for (uint t = 0; t < kTailLength; t++) {
            const uint  idx = (head + ring.count() - t) % ring.count();
            const float fade = 1.0f - static_cast<float>(t) / kTailLength;
            // Quadratic falloff reads far smoother than linear once gamma is
            // applied on top.
            ring.set_hsv(idx, hue + static_cast<float>(t) * 4.0f, 1.0f, fade * fade);
        }
        ring.show();

        head = (head + 1) % ring.count();
        hue += kHueStepPerFrame;

        // --- digits: seconds, separator blinking twice a second ---
        if (frame % (kFramesPerSecond / 2) == 0) {
            digits.show_time(static_cast<int>(seconds / 60), static_cast<int>(seconds % 60),
                             (frame / (kFramesPerSecond / 2)) % 2 == 0);
            digits.show();
        }

        if (frame % kFramesPerSecond == 0) {
            char line[17];
            snprintf(line, sizeof(line), "%3lu + %3lu mA", (unsigned long)ring.last_frame_ma(),
                     (unsigned long)digit_pixels.last_frame_ma());
            if (lcd_ok) lcd.print_line(1, line);
            printf("t=%lus  ring %lu mA  digits %lu mA\n", (unsigned long)seconds,
                   (unsigned long)ring.last_frame_ma(),
                   (unsigned long)digit_pixels.last_frame_ma());
            led_set(seconds % 2 == 0);
            seconds++;
        }

        frame++;
        sleep_ms(kFrameMs);
    }
}
