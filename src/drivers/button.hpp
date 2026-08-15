// Debounced push button with an optional lamp.
//
// For the arcade button, which is the whole user interface for a five-year-old
// — one big obvious thing to hit.
//
// Wired active-low: the switch shorts the pin to GND and an internal pull-up
// holds it high otherwise, so no external resistor is needed.

#pragma once

#include <cstdint>

#include "pico/stdlib.h"

namespace drivers {

class Button {
public:
    static constexpr uint kNoLamp = 0xFFFFFFFFu;

    explicit Button(uint pin, uint lamp_pin = kNoLamp);

    void init();

    // Call every time round the main loop.
    void poll();

    bool is_pressed() const { return pressed_; }

    // True exactly once per press. Reading it consumes the event, so a slow
    // main loop cannot lose a click.
    bool take_click();

    // True once when the button has been held this long. Useful later for a
    // grown-up settings mode a child will not stumble into.
    bool take_long_press(uint32_t hold_ms = 1500);

    void set_lamp(bool on);
    bool lamp_on() const { return lamp_on_; }

private:
    uint pin_;
    uint lamp_pin_;

    bool pressed_ = false;
    bool raw_last_ = false;
    bool click_pending_ = false;
    bool long_press_fired_ = false;
    bool lamp_on_ = false;

    absolute_time_t last_change_{};
    absolute_time_t pressed_since_{};
};

}  // namespace drivers
