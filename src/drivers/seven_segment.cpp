#include "drivers/seven_segment.hpp"

namespace drivers {
namespace {

// Bit 0 = segment a, bit 1 = b, ... bit 6 = g, matching the pixel offsets.
constexpr uint8_t kDigitFont[10] = {
    0x3F,  // 0  abcdef
    0x06,  // 1  bc
    0x5B,  // 2  abdeg
    0x4F,  // 3  abcdg
    0x66,  // 4  bcfg
    0x6D,  // 5  acdfg
    0x7D,  // 6  acdefg
    0x07,  // 7  abc
    0x7F,  // 8  abcdefg
    0x6F,  // 9  abcdfg
};

constexpr uint8_t kBlank = 0x00;
constexpr uint8_t kDash = 0x40;  // segment g alone

}  // namespace

SevenSegment::SevenSegment(NeoPixels &pixels, uint digit_count)
    : pixels_(pixels), digit_count_(digit_count) {}

void SevenSegment::set_colour(uint8_t r, uint8_t g, uint8_t b) {
    r_ = r;
    g_ = g;
    b_ = b;
}

void SevenSegment::clear() { pixels_.clear(); }

void SevenSegment::apply_glyph(uint index, uint8_t segment_bits) {
    if (index >= digit_count_) return;
    const uint base = index * kPixelsPerDigit;
    for (uint s = 0; s < kSegmentsPerDigit; s++) {
        const bool on = (segment_bits >> s) & 1;
        if (on) {
            pixels_.set(base + s, r_, g_, b_);
        } else {
            pixels_.set(base + s, 0, 0, 0);
        }
    }
}

void SevenSegment::set_segments(uint index, uint8_t segment_bits) {
    apply_glyph(index, segment_bits);
}

void SevenSegment::set_digit(uint index, int value) {
    apply_glyph(index, (value >= 0 && value <= 9) ? kDigitFont[value] : kBlank);
}

void SevenSegment::set_dot(uint index, bool on) {
    if (index >= digit_count_) return;
    const uint pixel = index * kPixelsPerDigit + kDotOffset;
    if (on) {
        pixels_.set(pixel, r_, g_, b_);
    } else {
        pixels_.set(pixel, 0, 0, 0);
    }
}

void SevenSegment::show_number(int value, bool leading_zeros) {
    const bool negative = value < 0;
    if (negative) value = -value;

    // Too big to display at all — dashes are clearer than showing the wrong
    // number by silently truncating.
    int limit = 1;
    for (uint i = 0; i < digit_count_; i++) limit *= 10;
    if (value >= limit) {
        for (uint i = 0; i < digit_count_; i++) apply_glyph(i, kDash);
        return;
    }

    for (uint i = 0; i < digit_count_; i++) {
        const uint pos = digit_count_ - 1 - i;  // rightmost digit first
        const int  digit = value % 10;
        const bool significant = (value > 0) || (i == 0);

        if (significant || leading_zeros) {
            set_digit(pos, digit);
        } else if (negative && value == 0) {
            apply_glyph(pos, kDash);  // the minus sign
            value = 0;
            continue;
        } else {
            set_digit(pos, -1);  // blank
        }
        value /= 10;
    }
}

void SevenSegment::show_time(int hours, int minutes, bool separator) {
    set_digit(0, (hours / 10) % 10);
    set_digit(1, hours % 10);
    set_digit(2, (minutes / 10) % 10);
    set_digit(3, minutes % 10);

    // No colon on this display, so digit 1's dot stands in for it.
    for (uint i = 0; i < digit_count_; i++) set_dot(i, false);
    set_dot(1, separator);
}

}  // namespace drivers
