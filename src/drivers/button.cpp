#include "drivers/button.hpp"

#include <cmath>

#include "hardware/gpio.h"
#include "hardware/pwm.h"

namespace drivers {
namespace {

// Arcade microswitches bounce noticeably more than a small tactile switch,
// and a five-year-old hits them hard.
constexpr int64_t kDebounceUs = 8000;

}  // namespace

Button::Button(uint pin, uint lamp_pin) : pin_(pin), lamp_pin_(lamp_pin) {}

void Button::init() {
    gpio_init(pin_);
    gpio_set_dir(pin_, GPIO_IN);
    gpio_pull_up(pin_);

    if (lamp_pin_ != kNoLamp) {
        gpio_set_function(lamp_pin_, GPIO_FUNC_PWM);
        // The lamp is a low-current LED with its own series resistor, driven
        // straight from the pin — no transistor on the final board, which is
        // two fewer parts to solder into a mint tin. RP2040 pads default to
        // 4 mA; 12 mA is the maximum and keeps the output from sagging.
        gpio_set_drive_strength(lamp_pin_, GPIO_DRIVE_STRENGTH_12MA);
        lamp_slice_ = pwm_gpio_to_slice_num(lamp_pin_);
        pwm_config cfg = pwm_get_default_config();
        // 8-bit wrap at ~1 kHz: fast enough that no one sees flicker, slow
        // enough that a switching transistor stays comfortably in its depth.
        pwm_config_set_clkdiv(&cfg, 488.0f);
        pwm_config_set_wrap(&cfg, 255);
        pwm_init(lamp_slice_, &cfg, true);
        pwm_set_gpio_level(lamp_pin_, 0);
    }

    raw_last_ = pressed_ = false;
    last_change_ = get_absolute_time();
}

void Button::poll() {
    const bool now = !gpio_get(pin_);  // active low

    if (now != raw_last_) {
        raw_last_ = now;
        last_change_ = get_absolute_time();
        return;  // wait for it to settle before believing it
    }

    if (now != pressed_ && absolute_time_diff_us(last_change_, get_absolute_time()) > kDebounceUs) {
        pressed_ = now;
        if (pressed_) {
            click_pending_ = true;
            pressed_since_ = get_absolute_time();
            long_press_fired_ = false;
        }
    }
}

bool Button::take_click() {
    if (!click_pending_) return false;
    click_pending_ = false;
    return true;
}

bool Button::take_long_press(uint32_t hold_ms) {
    if (!pressed_ || long_press_fired_) return false;
    if (absolute_time_diff_us(pressed_since_, get_absolute_time()) <
        static_cast<int64_t>(hold_ms) * 1000) {
        return false;
    }
    long_press_fired_ = true;
    return true;
}

void Button::set_lamp(bool on) { set_lamp_brightness(on ? 255 : 0); }

void Button::set_lamp_brightness(uint8_t level) {
    lamp_level_ = level;
    if (lamp_pin_ == kNoLamp) return;
    // Same reasoning as the NeoPixels: LED output is linear in duty cycle but
    // the eye is not, so without gamma a "breath" spends most of its time
    // looking fully lit.
    const uint16_t duty = static_cast<uint16_t>(
        powf(static_cast<float>(level) / 255.0f, 2.2f) * 255.0f + 0.5f);
    pwm_set_gpio_level(lamp_pin_, duty);
}

void Button::breathe_lamp(uint32_t period_ms, uint8_t min_level, uint8_t max_level) {
    const uint32_t now = to_ms_since_boot(get_absolute_time()) % period_ms;
    const float    phase = static_cast<float>(now) / static_cast<float>(period_ms);
    // Raised cosine: starts dim, swells, falls back, with no discontinuity at
    // the wrap point.
    const float    wave = 0.5f - 0.5f * cosf(phase * 2.0f * 3.14159265f);
    set_lamp_brightness(static_cast<uint8_t>(min_level + wave * (max_level - min_level)));
}

}  // namespace drivers
