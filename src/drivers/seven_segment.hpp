// Four-digit NeoPixel seven-segment display.
//
// Layout, taken from the Siebensegment Uhr project rather than guessed: each
// digit is eight consecutive pixels, offsets 0-6 being segments a..g in order
// and offset 7 the digit's dot.
//
//        a           pixel offsets within a digit
//      ┌───┐         0=a  1=b  2=c  3=d
//    f │   │ b       4=e  5=f  6=g  7=dot
//      ├─g─┤
//    e │   │ c
//      └───┘ ·dot
//        d
//
// This owns a NeoPixels chain of its own rather than sharing the ring's, so
// digits can be redrawn once a second while the ring animates at 25 fps.

#pragma once

#include <cstdint>

#include "drivers/neopixels.hpp"

namespace drivers {

class SevenSegment {
public:
    static constexpr uint kSegmentsPerDigit = 7;
    static constexpr uint kPixelsPerDigit = 8;
    static constexpr uint kDotOffset = 7;

    SevenSegment(NeoPixels &pixels, uint digit_count = 4);

    void set_colour(uint8_t r, uint8_t g, uint8_t b);

    void clear();

    // value 0-9 lights that glyph; anything else blanks the digit.
    void set_digit(uint index, int value);
    void set_dot(uint index, bool on);

    // Raw segment control: bit 0 = a, bit 1 = b, ... bit 6 = g. Used by the
    // bring-up test that walks one segment at a time to confirm the mapping.
    void set_segments(uint index, uint8_t segment_bits);

    // Right-aligned. Without leading_zeros, unused digits on the left stay
    // dark; a value too large for the display shows dashes.
    void show_number(int value, bool leading_zeros = false);

    // HH:MM across four digits, with the separator on digit 1's dot so it can
    // be blinked once a second.
    void show_time(int hours, int minutes, bool separator = true);

    // Pushes the frame. Call after any set_/show_ calls.
    void show() { pixels_.show(); }

    NeoPixels &pixels() { return pixels_; }

private:
    void apply_glyph(uint index, uint8_t segment_bits);

    NeoPixels &pixels_;
    uint       digit_count_;
    uint8_t    r_ = 255, g_ = 120, b_ = 0;  // a warm amber by default
};

}  // namespace drivers
