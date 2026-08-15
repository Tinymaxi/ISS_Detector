// WS2812 chain driven by PIO.
//
// The ring and the seven-segment sit on one data line — ring DOUT into
// seg DIN — so they are a single chain to this driver. Ring pixels occupy
// indices [0, NEOPIXEL_RING), the seven-segment follows.
//
// Two things happen between set() and the wire:
//
//   Gamma correction, because WS2812 brightness is linear in PWM duty but the
//   eye is not. Without it a "half brightness" pixel looks nearly full.
//
//   A current budget. 56 pixels at full white would draw about 3.4 A, far past
//   what the supply or the USB-C breakout will give. Rather than capping
//   brightness globally — which would dim a single lit pixel for no reason —
//   the whole frame is measured and scaled only when it would exceed budget.
//   A lone ISS pixel stays bright; a full-ring white flash gets dimmed.

#pragma once

#include <cstdint>

#include "hardware/pio.h"
#include "pico/stdlib.h"

namespace drivers {

class NeoPixels {
public:
    static constexpr uint kMaxPixels = 64;

    // 400 mA suits bring-up over a laptop's USB port. Raise it once the box
    // runs from the 5 V Raspberry Pi supply.
    NeoPixels(uint pin, uint count, uint32_t current_budget_ma = 400);

    void init(PIO pio = pio0);

    void set(uint index, uint8_t r, uint8_t g, uint8_t b);
    void set_hsv(uint index, float hue_deg, float sat = 1.0f, float val = 1.0f);
    void fill(uint8_t r, uint8_t g, uint8_t b);
    void clear();

    // Multiplies everything by v/255 before gamma. Use for fades; the current
    // budget is enforced separately and always.
    void set_master_brightness(uint8_t v) { master_ = v; }
    void set_current_budget_ma(uint32_t ma) { budget_ma_ = ma; }

    // Estimated draw of the last frame actually sent, after any scaling.
    uint32_t last_frame_ma() const { return last_ma_; }

    // Applies brightness, gamma and the current budget, then pushes the frame.
    void show();

    uint count() const { return count_; }

private:
    struct Rgb {
        uint8_t r, g, b;
    };

    PIO      pio_ = nullptr;
    uint     sm_ = 0;
    uint     offset_ = 0;
    uint     pin_;
    uint     count_;
    uint32_t budget_ma_;
    uint8_t  master_ = 255;
    uint32_t last_ma_ = 0;

    Rgb     buf_[kMaxPixels] = {};
    uint8_t gamma_[256] = {};
};

}  // namespace drivers
