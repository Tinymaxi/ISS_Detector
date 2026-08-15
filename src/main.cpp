// Die ISS-Box — Stage 8: arcade button and buzzer
//
// Turn the knob to pick a tune, press the arcade button to play it. The ring
// keeps animating throughout, which is the point: the buzzer is driven by a
// state machine rather than by sleeping, so sound and light run together.

#include <cstdio>

#include "board_config.hpp"
#include "drivers/button.hpp"
#include "drivers/buzzer.hpp"
#include "drivers/i2c_bus.hpp"
#include "drivers/lcd1602_rgb.hpp"
#include "drivers/neopixels.hpp"
#include "drivers/rotary_encoder.hpp"
#include "drivers/seven_segment.hpp"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

namespace {

constexpr uint kPollMs = 2;

struct Tune {
    const char *name;
    const drivers::Buzzer::Note *(*get)(size_t &);
};

const Tune kTunes[] = {
    {"Start", drivers::Buzzer::startup},
    {"ISS-Alarm", drivers::Buzzer::iss_alert},
    {"Fanfare", drivers::Buzzer::fanfare},
    {"Pups", drivers::Buzzer::raspberry},
    {"Klick", drivers::Buzzer::click},
};
constexpr int kTuneCount = sizeof(kTunes) / sizeof(kTunes[0]);

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

    printf("\n=== ISS-Box Stage 8: button + buzzer ===\n");
    printf("cyw43_arch_init: %s\n", cyw43_arch_init() == 0 ? "ok" : "FAILED");

    drivers::i2c_bus().init();

    drivers::Lcd1602Rgb lcd(drivers::i2c_bus());
    const bool          lcd_ok = lcd.init();
    lcd.set_backlight(40, 40, 80);

    drivers::NeoPixels ring(board::PIN_RING, board::RING_PIXELS, 300);
    ring.init(pio0);

    drivers::NeoPixels digit_pixels(board::PIN_DIGITS, board::DIGIT_PIXELS_FITTED, 300);
    digit_pixels.init(pio0);
    digit_pixels.set_master_brightness(120);
    drivers::SevenSegment digits(digit_pixels, board::DIGITS_FITTED);

    drivers::RotaryEncoder encoder(board::PIN_ENCODER_CLK, board::PIN_ENCODER_SW);
    encoder.init(pio1);

    drivers::Button button(board::PIN_BUTTON, board::PIN_BUTTON_LAMP);
    button.init();

    drivers::Buzzer buzzer(board::PIN_BUZZER);
    buzzer.init();

    printf("buzzer GP%u, button GP%u, lamp GP%u\n", board::PIN_BUZZER, board::PIN_BUTTON,
           board::PIN_BUTTON_LAMP);

    // Announce ourselves, non-blocking — the ring starts animating immediately.
    size_t len = 0;
    const auto *melody = drivers::Buzzer::startup(len);
    buzzer.play(melody, len);

    int      selection = 0;
    uint32_t frame = 0;
    float    hue = 0.0f;
    uint     head = 0;
    bool     dirty = true;

    while (true) {
        encoder.poll();
        button.poll();
        buzzer.update();

        const int delta = encoder.take_delta();
        if (delta != 0) {
            selection = ((selection + delta) % kTuneCount + kTuneCount) % kTuneCount;
            printf("selection -> %d (%s)\n", selection, kTunes[selection].name);
            dirty = true;
        }

        // The encoder's own switch and the arcade button both play it, so the
        // buzzer can be tested before the arcade button is wired.
        if (button.take_click() || encoder.take_click()) {
            size_t      n = 0;
            const auto *m = kTunes[selection].get(n);
            buzzer.play(m, n);
            printf("play: %s\n", kTunes[selection].name);
            dirty = true;
        }

        // Lamp follows the sound, so the button glows while it sings.
        button.set_lamp(buzzer.is_playing());

        if (frame % 20 == 0) {  // ~25 fps at a 2 ms poll
            ring.clear();
            if (buzzer.is_playing()) {
                // Full ring pulsing while a tune plays.
                for (uint i = 0; i < ring.count(); i++) {
                    ring.set_hsv(i, hue + static_cast<float>(i) * 6.0f, 1.0f, 0.7f);
                }
            } else {
                // Idle: a comet, plus a marker showing the selected tune.
                for (uint t = 0; t < 6; t++) {
                    const uint  idx = (head + ring.count() - t) % ring.count();
                    const float fade = 1.0f - static_cast<float>(t) / 6.0f;
                    ring.set_hsv(idx, 200.0f, 0.8f, fade * fade * 0.5f);
                }
                ring.set(static_cast<uint>(selection) * 4, 255, 180, 0);
            }
            ring.show();

            head = (head + 1) % ring.count();
            hue += 6.0f;

            digits.clear();
            digits.set_digit(0, selection);
            digits.set_dot(0, buzzer.is_playing());
            digits.show();
        }

        if (dirty && lcd_ok) {
            lcd.print_line(0, kTunes[selection].name);
            lcd.print_line(1, buzzer.is_playing() ? "spielt..." : "Knopf druecken");
            dirty = false;
        }

        frame++;
        sleep_ms(kPollMs);
    }
}
