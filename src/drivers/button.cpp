#include "drivers/button.hpp"

#include "hardware/gpio.h"

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
        gpio_init(lamp_pin_);
        gpio_set_dir(lamp_pin_, GPIO_OUT);
        gpio_put(lamp_pin_, 0);
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

void Button::set_lamp(bool on) {
    lamp_on_ = on;
    if (lamp_pin_ != kNoLamp) gpio_put(lamp_pin_, on);
}

}  // namespace drivers
