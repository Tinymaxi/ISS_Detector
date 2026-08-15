#include "drivers/gas_sensor.hpp"

#include <cmath>

#include "hardware/adc.h"
#include "hardware/gpio.h"

namespace drivers {
namespace {

constexpr int   kOversample = 16;
constexpr float kAdcRefVolts = 3.3f;
constexpr float kAdcFullScale = 4095.0f;

// How fast the baseline chases the current reading.
//
// Size this against the ACTUAL update rate, not an assumed one: update() is
// called every time round the main loop, which polls at 2 ms, so ~500 times a
// second. The first version used 0.0008 on the assumption of ~20 Hz, giving a
// 2.5 second time constant — fast enough to swallow a slow fart into its own
// reference before it registered.
//
// alpha = 1 / (rate_hz * tau_seconds), for tau = 90 s at 500 Hz.
constexpr float kBaselineAlpha = 1.0f / (500.0f * 90.0f);

}  // namespace

GasSensor::GasSensor(uint adc_pin, uint adc_channel, uint en_pin, float r_top, float r_bottom)
    : adc_pin_(adc_pin), adc_channel_(adc_channel), en_pin_(en_pin),
      divider_scale_((r_top + r_bottom) / r_bottom) {}

void GasSensor::init() {
    adc_init();
    adc_gpio_init(adc_pin_);

    gpio_init(en_pin_);
    gpio_set_dir(en_pin_, GPIO_OUT);
    set_heater(true);
}

void GasSensor::set_heater(bool on) {
    // Inverted: HIGH disables.
    gpio_put(en_pin_, on ? 0 : 1);
}

uint16_t GasSensor::read_raw() {
    adc_select_input(adc_channel_);
    uint32_t total = 0;
    for (int i = 0; i < kOversample; i++) {
        total += adc_read();
    }
    return static_cast<uint16_t>(total / kOversample);
}

float GasSensor::read_sensor_volts() {
    const float at_pin = static_cast<float>(read_raw()) * kAdcRefVolts / kAdcFullScale;
    return at_pin * divider_scale_;
}

void GasSensor::update() {
    latest_v_ = read_sensor_volts();

    if (!baseline_ready_) {
        baseline_v_ = latest_v_;
        baseline_ready_ = true;
        return;
    }
    if (baseline_held_) return;

    // Chase downward quickly but upward slowly. Air clearing should be
    // followed promptly; a rising reading is the signal itself and must not be
    // absorbed into the reference.
    const float alpha = (latest_v_ < baseline_v_) ? kBaselineAlpha * 8.0f : kBaselineAlpha;
    baseline_v_ += alpha * (latest_v_ - baseline_v_);
}

float GasSensor::excess_volts() const {
    const float excess = latest_v_ - baseline_v_;
    return excess > 0.0f ? excess : 0.0f;
}

void GasSensor::reset_baseline() {
    baseline_v_ = latest_v_;
    baseline_ready_ = true;
}

int GasSensor::score(float excess_v, float full_scale_volts) const {
    if (full_scale_volts <= 0.0f) return 0;
    float frac = excess_v / full_scale_volts;
    if (frac < 0.0f) frac = 0.0f;

    // Square root curve: generous at the bottom so a small puff still scores
    // something worth celebrating, compressed at the top so 99 stays earnable
    // but not automatic.
    frac = sqrtf(frac);
    if (frac > 1.0f) frac = 1.0f;
    return static_cast<int>(frac * 99.0f + 0.5f);
}

}  // namespace drivers
