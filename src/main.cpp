// Die ISS-Box — Stage 7: rotary encoder
//
// Turning the knob moves a marker round the ring and changes a number on the
// LCD; the push switch resets it. Direction and detent counting are checked by
// turning it and watching whether the numbers agree with your hand.

#include <cstdio>

#include "board_config.hpp"
#include "drivers/i2c_bus.hpp"
#include "drivers/lcd1602_rgb.hpp"
#include "drivers/neopixels.hpp"
#include "drivers/rotary_encoder.hpp"
#include "drivers/seven_segment.hpp"
#include "drivers/sht_sensor.hpp"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "tools/i2c_scan.hpp"

namespace {

constexpr uint kPollMs = 2;   // encoder debounce wants frequent polling
constexpr uint kFrameMs = 40;

bool led_init() { return cyw43_arch_init() == 0; }

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

    printf("\n=== ISS-Box Stage 7: rotary encoder ===\n");
    // The binary shrank by ~230 KB when the CYW43 firmware stopped being
    // embedded, so check the wireless chip really does come up. WiFi at
    // Stage 10 depends on this, and a silent failure now would be found late.
    printf("cyw43_arch_init: %s\n", led_init() ? "ok" : "FAILED");

    drivers::i2c_bus().init();
    tools::i2c_scan();

    drivers::Lcd1602Rgb lcd(drivers::i2c_bus());
    const bool          lcd_ok = lcd.init();
    lcd.set_backlight(0, 60, 60);
    lcd.print_line(0, "Drehen + Klick");

    drivers::ShtSensor sht(drivers::i2c_bus());
    sht.init();

    // NeoPixels take PIO0. The quadrature decoder must sit at instruction
    // address 0 of its PIO, which WS2812 already occupies here, so the encoder
    // goes on PIO1.
    drivers::NeoPixels ring(board::PIN_RING, board::RING_PIXELS, 300);
    ring.init(pio0);

    drivers::NeoPixels digit_pixels(board::PIN_DIGITS, board::DIGIT_PIXELS_FITTED, 300);
    digit_pixels.init(pio0);
    digit_pixels.set_master_brightness(120);
    drivers::SevenSegment digits(digit_pixels, board::DIGITS_FITTED);

    drivers::RotaryEncoder encoder(board::PIN_ENCODER_CLK, board::PIN_ENCODER_SW);
    encoder.init(pio1);
    printf("encoder: CLK=GP%u DT=GP%u SW=GP%u on pio1\n", board::PIN_ENCODER_CLK,
           board::PIN_ENCODER_DT, board::PIN_ENCODER_SW);

    int      value = 0;
    uint32_t clicks = 0;
    uint32_t frame = 0;
    bool     dirty = true;

    while (true) {
        encoder.poll();

        const int delta = encoder.take_delta();
        if (delta != 0) {
            value += delta;
            printf("turn %+d -> %d\n", delta, value);
            dirty = true;
        }

        if (encoder.take_click()) {
            clicks++;
            value = 0;
            printf("click %lu (reset)\n", (unsigned long)clicks);
            dirty = true;
        }

        // Redraw only on change, so the ring is not rewritten 500 times a
        // second while the knob sits still.
        if (dirty || frame % (kFrameMs / kPollMs) == 0) {
            // Marker at the current position, wrapping like a compass — a
            // direct preview of how the ISS bearing will be drawn.
            const uint pos =
                static_cast<uint>(((value % static_cast<int>(ring.count())) + ring.count()) %
                                  static_cast<int>(ring.count()));
            ring.clear();
            for (uint i = 0; i < ring.count(); i++) ring.set(i, 0, 0, 6);  // dim backdrop
            ring.set(pos, 255, 255, 255);
            ring.show();

            digits.clear();
            digits.set_digit(0, ((value % 10) + 10) % 10);
            digits.set_dot(0, encoder.is_pressed());
            digits.show();
        }

        if (dirty && lcd_ok) {
            char line[17];
            snprintf(line, sizeof(line), "Pos %-4d Clk %lu", value, (unsigned long)clicks);
            lcd.print_line(1, line);
            dirty = false;
        }

        frame++;
        sleep_ms(kPollMs);
    }
}
