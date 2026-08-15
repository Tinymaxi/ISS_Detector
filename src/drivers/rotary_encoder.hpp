// Rotary encoder with push switch (KY-040 style: CLK, DT, SW, +, GND).
//
// Decoding runs in PIO, using the SDK's quadrature_encoder program, so no step
// is ever missed regardless of what the main loop is doing.
//
// Two constraints come from that program and are easy to trip over:
//
//   It declares `.origin 0` and uses computed jumps into a table, so it must
//   live at instruction address 0. WS2812 already holds address 0 of PIO0,
//   which is why the encoder defaults to PIO1.
//
//   CLK and DT must be consecutive GPIO with CLK first — the program reads
//   them as a two-bit field from a base pin.
//
// Power the module from 3V3, never 5 V. Its onboard pull-ups sit on CLK and
// DT, and the RP2040 is not 5 V tolerant.

#pragma once

#include <cstdint>

#include "hardware/pio.h"
#include "pico/stdlib.h"

namespace drivers {

class RotaryEncoder {
public:
    // Most KY-040 style encoders emit four quadrature transitions per physical
    // click, so raw counts are divided by this to get detents.
    static constexpr int kCountsPerDetent = 4;

    RotaryEncoder(uint clk_pin, uint sw_pin);

    void init(PIO pio = pio1);

    // Call regularly from the main loop; debounces the push switch.
    void poll();

    // Detents turned since the last call to this function. Positive is
    // clockwise. Reading it consumes the delta.
    int take_delta();

    // Cumulative detent position since boot.
    int detents() const { return detents_; }

    // Current debounced state of the push switch.
    bool is_pressed() const { return pressed_; }

    // True exactly once per press. Reading it consumes the event, so a slow
    // main loop still never loses a click.
    bool take_click();

private:
    uint     clk_pin_;
    uint     sw_pin_;
    PIO      pio_ = nullptr;
    uint     sm_ = 0;

    int32_t raw_ = 0;
    int     detents_ = 0;
    int     last_reported_detents_ = 0;

    bool            pressed_ = false;
    bool            raw_last_ = false;
    bool            click_pending_ = false;
    absolute_time_t last_change_{};
};

}  // namespace drivers
