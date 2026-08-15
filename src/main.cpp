// Die ISS-Box — Stage 9: gas sensor
//
// A live view of what the MiCS-5524 is doing: the voltage now, the tracked
// baseline, and how far above it we are. The ring shows the excess as a bar so
// it can be seen from across the room while breathing on the sensor.
//
// Press the button (or the encoder) to freeze the baseline and take a scored
// measurement — a first cut of the Pups-o-Meter.

#include <cstdio>

#include "board_config.hpp"
#include "drivers/button.hpp"
#include "drivers/buzzer.hpp"
#include "drivers/gas_sensor.hpp"
#include "drivers/i2c_bus.hpp"
#include "drivers/lcd1602_rgb.hpp"
#include "drivers/neopixels.hpp"
#include "drivers/rotary_encoder.hpp"
#include "drivers/seven_segment.hpp"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

namespace {

constexpr uint     kPollMs = 2;
constexpr uint32_t kMeasureMs = 5000;  // how long a scored measurement runs

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
    cyw43_arch_init();

    printf("\n=== ISS-Box Stage 9: gas sensor ===\n");

    drivers::i2c_bus().init();

    drivers::Lcd1602Rgb lcd(drivers::i2c_bus());
    const bool          lcd_ok = lcd.init();
    lcd.set_backlight(120, 200, 255);

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

    // 10k from A0 to the ADC pin; 10k + 4.7k + 470 in series to ground.
    //
    // That puts 5 V at 3.01 V, using 91% of the ADC range. The remaining
    // headroom is not waste: USB may legitimately supply up to 5.25 V, which
    // would put this at 3.16 V — still under the 3.3 V rail. A divider tuned
    // to use the last few percent would clip on a hot USB port.
    drivers::GasSensor gas(board::PIN_GAS_AO, board::GAS_ADC_CHANNEL, board::PIN_GAS_EN,
                           10000.0f, 15170.0f);
    gas.init();
    printf("gas: A0 GP%u (ADC%u), EN GP%u, heater on\n", board::PIN_GAS_AO,
           board::GAS_ADC_CHANNEL, board::PIN_GAS_EN);

    if (lcd_ok) lcd.print_line(0, "Sensor waermt");
    sleep_ms(1500);  // the heater needs a moment before readings mean anything
    gas.update();
    gas.reset_baseline();

    bool            measuring = false;
    absolute_time_t measure_end{};
    float           peak_excess = 0.0f;
    int             last_score = -1;
    uint32_t        frame = 0;

    while (true) {
        encoder.poll();
        button.poll();
        buzzer.update();
        gas.update();

        if (!measuring && (button.take_click() || encoder.take_click())) {
            measuring = true;
            peak_excess = 0.0f;
            gas.hold_baseline(true);  // keep the puff out of its own reference
            measure_end = make_timeout_time_ms(kMeasureMs);
            buzzer.beep(1200, 80);
            printf("--- measuring for %lu ms ---\n", (unsigned long)kMeasureMs);
        }

        if (measuring) {
            const float e = gas.excess_volts();
            if (e > peak_excess) peak_excess = e;

            if (time_reached(measure_end)) {
                measuring = false;
                gas.hold_baseline(false);
                last_score = gas.score(peak_excess);
                printf("RESULT peak %+.3f V -> score %d\n", (double)peak_excess, last_score);

                size_t      n = 0;
                const auto *m = last_score >= 50 ? drivers::Buzzer::fanfare(n)
                                                 : drivers::Buzzer::raspberry(n);
                buzzer.play(m, n);
            }
        }

        if (measuring) {
            button.set_lamp_brightness(255);
        } else if (!buzzer.is_playing()) {
            button.breathe_lamp();
        }

        if (frame % 20 == 0) {  // ~25 fps
            const float excess = gas.excess_volts();
            const int   live_score = gas.score(measuring ? peak_excess : excess);

            // Ring as a bar graph, green at the bottom running to red.
            ring.clear();
            const uint lit = static_cast<uint>(
                (static_cast<float>(live_score) / 99.0f) * static_cast<float>(ring.count()) + 0.5f);
            for (uint i = 0; i < lit && i < ring.count(); i++) {
                const float hue =
                    120.0f * (1.0f - static_cast<float>(i) / static_cast<float>(ring.count() - 1));
                ring.set_hsv(i, hue, 1.0f, 0.6f);
            }
            ring.show();

            digits.clear();
            digits.set_digit(0, (live_score / 10) % 10);
            digits.set_dot(0, measuring);
            digits.show();
        }

        if (frame % 250 == 0) {  // twice a second
            printf("now %.3f V   base %.3f V   excess %+.3f V   score %2d%s\n",
                   (double)gas.latest_volts(), (double)gas.baseline_volts(),
                   (double)gas.excess_volts(), gas.score(gas.excess_volts()),
                   measuring ? "   [measuring]" : "");

            if (lcd_ok) {
                char line[17];
                snprintf(line, sizeof(line), "%.2fV b%.2fV", (double)gas.latest_volts(),
                         (double)gas.baseline_volts());
                lcd.print_line(0, line);
                if (measuring) {
                    snprintf(line, sizeof(line), "messen... %2d", gas.score(peak_excess));
                } else if (last_score >= 0) {
                    snprintf(line, sizeof(line), "letzte: %2d", last_score);
                } else {
                    snprintf(line, sizeof(line), "Knopf = messen");
                }
                lcd.print_line(1, line);
            }
        }

        frame++;
        sleep_ms(kPollMs);
    }
}
