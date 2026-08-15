#include "drivers/rotary_encoder.hpp"

#include "hardware/gpio.h"
#include "quadrature_encoder.pio.h"

namespace drivers {
namespace {

// Mechanical contacts bounce for a few milliseconds. Long enough to reject
// that, short enough that a deliberate press still feels instant.
constexpr int64_t kDebounceUs = 5000;

}  // namespace

RotaryEncoder::RotaryEncoder(uint clk_pin, uint sw_pin) : clk_pin_(clk_pin), sw_pin_(sw_pin) {}

void RotaryEncoder::init(PIO pio) {
    pio_ = pio;
    sm_ = pio_claim_unused_sm(pio_, true);
    // The program's .origin 0 means pio_add_program places it at address 0;
    // nothing else may occupy that address on this PIO.
    pio_add_program(pio_, &quadrature_encoder_program);
    // Zero sample rate = run at full clock. A hand-turned knob is nowhere near
    // fast enough to need throttling, and this keeps the decode exact.
    quadrature_encoder_program_init(pio_, sm_, clk_pin_, 0);

    // The KY-040 pulls up CLK and DT on the module, but usually leaves SW
    // floating, so it needs an internal pull-up. Pressing pulls it to GND.
    gpio_init(sw_pin_);
    gpio_set_dir(sw_pin_, GPIO_IN);
    gpio_pull_up(sw_pin_);

    raw_ = quadrature_encoder_get_count(pio_, sm_);
    detents_ = raw_ / kCountsPerDetent;
    last_reported_detents_ = detents_;

    raw_last_ = pressed_ = false;
    last_change_ = get_absolute_time();
}

void RotaryEncoder::poll() {
    if (pio_ == nullptr) return;

    raw_ = quadrature_encoder_get_count(pio_, sm_);
    detents_ = raw_ / kCountsPerDetent;

    // Active low: the switch shorts to GND when pressed.
    const bool now = !gpio_get(sw_pin_);

    if (now != raw_last_) {
        raw_last_ = now;
        last_change_ = get_absolute_time();
        return;  // wait for it to settle before believing it
    }

    if (now != pressed_ && absolute_time_diff_us(last_change_, get_absolute_time()) > kDebounceUs) {
        pressed_ = now;
        if (pressed_) click_pending_ = true;  // latch the press edge
    }
}

int RotaryEncoder::take_delta() {
    const int delta = detents_ - last_reported_detents_;
    last_reported_detents_ = detents_;
    return delta;
}

bool RotaryEncoder::take_click() {
    if (!click_pending_) return false;
    click_pending_ = false;
    return true;
}

}  // namespace drivers
