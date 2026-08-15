#include "drivers/neopixels.hpp"

#include <cmath>

#include "hardware/clocks.h"
#include "ws2812.pio.h"

namespace drivers {
namespace {

// Rough datasheet figures for a WS2812: each of the three dies draws about
// 20 mA at full duty, plus a milliamp or so of quiescent draw per pixel.
constexpr float kMaPerChannelFull = 20.0f / 255.0f;
constexpr float kQuiescentMaPerPixel = 1.0f;

constexpr float kGamma = 2.6f;

}  // namespace

NeoPixels::NeoPixels(uint pin, uint count, uint32_t current_budget_ma)
    : pin_(pin), count_(count > kMaxPixels ? kMaxPixels : count),
      budget_ma_(current_budget_ma) {}

void NeoPixels::init(PIO pio) {
    pio_ = pio;
    sm_ = pio_claim_unused_sm(pio_, true);
    offset_ = pio_add_program(pio_, &ws2812_program);
    ws2812_program_init(pio_, sm_, offset_, pin_, 800000, false);

    for (int i = 0; i < 256; i++) {
        gamma_[i] = static_cast<uint8_t>(powf(i / 255.0f, kGamma) * 255.0f + 0.5f);
    }

    clear();
    show();
}

void NeoPixels::set(uint index, uint8_t r, uint8_t g, uint8_t b) {
    if (index >= count_) return;
    buf_[index] = {r, g, b};
}

void NeoPixels::set_hsv(uint index, float hue_deg, float sat, float val) {
    while (hue_deg < 0.0f) hue_deg += 360.0f;
    while (hue_deg >= 360.0f) hue_deg -= 360.0f;

    const float c = val * sat;
    const float h = hue_deg / 60.0f;
    const float x = c * (1.0f - fabsf(fmodf(h, 2.0f) - 1.0f));
    const float m = val - c;

    float rf = 0, gf = 0, bf = 0;
    switch (static_cast<int>(h)) {
        case 0: rf = c; gf = x; break;
        case 1: rf = x; gf = c; break;
        case 2: gf = c; bf = x; break;
        case 3: gf = x; bf = c; break;
        case 4: rf = x; bf = c; break;
        default: rf = c; bf = x; break;
    }

    set(index, static_cast<uint8_t>((rf + m) * 255.0f), static_cast<uint8_t>((gf + m) * 255.0f),
        static_cast<uint8_t>((bf + m) * 255.0f));
}

void NeoPixels::fill(uint8_t r, uint8_t g, uint8_t b) {
    for (uint i = 0; i < count_; i++) buf_[i] = {r, g, b};
}

void NeoPixels::clear() { fill(0, 0, 0); }

void NeoPixels::show() {
    if (pio_ == nullptr) return;

    // Brightness and gamma first, because what reaches the LED is what draws
    // the current — measuring the raw values would overestimate badly.
    uint8_t out[kMaxPixels][3];
    float   total_ma = kQuiescentMaPerPixel * static_cast<float>(count_);

    for (uint i = 0; i < count_; i++) {
        const uint8_t r = gamma_[buf_[i].r * master_ / 255];
        const uint8_t g = gamma_[buf_[i].g * master_ / 255];
        const uint8_t b = gamma_[buf_[i].b * master_ / 255];
        out[i][0] = r;
        out[i][1] = g;
        out[i][2] = b;
        total_ma += static_cast<float>(r + g + b) * kMaPerChannelFull;
    }

    // Scale the whole frame down only if it would exceed the budget, so a
    // single bright pixel is never dimmed on account of pixels that are off.
    float scale = 1.0f;
    if (total_ma > static_cast<float>(budget_ma_)) {
        scale = static_cast<float>(budget_ma_) / total_ma;
    }
    last_ma_ = static_cast<uint32_t>(total_ma * scale);

    for (uint i = 0; i < count_; i++) {
        const uint8_t r = static_cast<uint8_t>(out[i][0] * scale);
        const uint8_t g = static_cast<uint8_t>(out[i][1] * scale);
        const uint8_t b = static_cast<uint8_t>(out[i][2] * scale);
        // WS2812 wants GRB, left-aligned in the 32-bit word the PIO shifts out.
        const uint32_t grb = (static_cast<uint32_t>(g) << 24) |
                             (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(b) << 8);
        pio_sm_put_blocking(pio_, sm_, grb);
    }

    // The reset gap that latches the frame: WS2812 needs the line low for
    // >50 µs, and a little margin costs nothing at these frame rates.
    sleep_us(300);
}

}  // namespace drivers
