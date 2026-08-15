// Die ISS-Box — Stage 5: the NeoPixel ring
//
// A comet orbits the ring with a fading tail, hue drifting as it goes. This is
// a deliberate preview of ISS mode, where a single bright pixel marks the
// station's bearing against a dim background.
//
// The LCD reports the estimated current of each frame, so the power budget is
// visible rather than theoretical.

#include <cstdio>

#include "board_config.hpp"
#include "drivers/i2c_bus.hpp"
#include "drivers/lcd1602_rgb.hpp"
#include "drivers/neopixels.hpp"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "tools/i2c_scan.hpp"

namespace {

constexpr uint  kTailLength = 8;
constexpr uint  kFrameMs = 40;
constexpr float kHueStepPerFrame = 1.5f;

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

    printf("\n=== ISS-Box Stage 5 ===\n");
    printf("board: %s   sdk: %s\n", PICO_BOARD, PICO_SDK_VERSION_STRING);

    drivers::i2c_bus().init();
    tools::i2c_scan();

    drivers::Lcd1602Rgb lcd(drivers::i2c_bus());
    const bool lcd_ok = lcd.init();
    lcd.set_backlight(0, 40, 80);
    lcd.print_line(0, "Ring-Test");

    // Only the ring is wired so far; this becomes NEOPIXEL_TOTAL once the
    // seven-segment is chained onto the end.
    drivers::NeoPixels ring(board::PIN_NEOPIXEL, board::NEOPIXEL_RING);
    ring.init();
    printf("ring: %u pixels on GP%u\n", ring.count(), board::PIN_NEOPIXEL);

    uint     head = 0;
    float    hue = 0.0f;
    uint32_t frame = 0;

    while (true) {
        ring.clear();

        // Tail fades quadratically, which reads as a much smoother falloff
        // than linear once gamma correction is applied on top.
        for (uint t = 0; t < kTailLength; t++) {
            const uint  idx = (head + ring.count() - t) % ring.count();
            const float fade = 1.0f - static_cast<float>(t) / kTailLength;
            ring.set_hsv(idx, hue + static_cast<float>(t) * 4.0f, 1.0f, fade * fade);
        }
        ring.show();

        head = (head + 1) % ring.count();
        hue += kHueStepPerFrame;

        // The LCD is far slower than the ring, so refresh it about once a
        // second rather than every frame.
        if (frame % (1000 / kFrameMs) == 0) {
            char line[17];
            snprintf(line, sizeof(line), "%3lu mA  %2u px", (unsigned long)ring.last_frame_ma(),
                     ring.count());
            if (lcd_ok) lcd.print_line(1, line);
            printf("frame %lu  %lu mA\n", (unsigned long)frame,
                   (unsigned long)ring.last_frame_ma());
            led_set(frame % (2000 / kFrameMs) == 0);
        }

        frame++;
        sleep_ms(kFrameMs);
    }
}
