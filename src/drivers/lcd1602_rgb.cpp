#include "drivers/lcd1602_rgb.hpp"

#include <cstring>

#include "pico/stdlib.h"

namespace drivers {
namespace {

// HD44780 command set, as understood by the AiP31068.
constexpr uint8_t kClearDisplay = 0x01;
constexpr uint8_t kEntryModeSet = 0x04;
constexpr uint8_t kDisplayControl = 0x08;
constexpr uint8_t kFunctionSet = 0x20;

constexpr uint8_t kEntryLeft = 0x02;
constexpr uint8_t kDisplayOn = 0x04;
constexpr uint8_t kTwoLine = 0x08;

// Control bytes prefixing every I2C transfer to the character controller.
constexpr uint8_t kCtrlCommand = 0x80;
constexpr uint8_t kCtrlData = 0x40;

// PCA9633 registers driving the backlight.
constexpr uint8_t kRegMode1 = 0x00;
constexpr uint8_t kRegMode2 = 0x01;
constexpr uint8_t kRegBlue = 0x02;
constexpr uint8_t kRegGreen = 0x03;
constexpr uint8_t kRegRed = 0x04;
constexpr uint8_t kRegLedOut = 0x08;

}  // namespace

Lcd1602Rgb::Lcd1602Rgb(SoftI2C &bus, uint8_t lcd_addr, uint8_t rgb_addr)
    : bus_(bus), lcd_addr_(lcd_addr), rgb_addr_(rgb_addr) {}

bool Lcd1602Rgb::command(uint8_t value) {
    const uint8_t frame[2] = {kCtrlCommand, value};
    return bus_.write(lcd_addr_, frame, 2);
}

bool Lcd1602Rgb::write_char(uint8_t value) {
    const uint8_t frame[2] = {kCtrlData, value};
    return bus_.write(lcd_addr_, frame, 2);
}

bool Lcd1602Rgb::set_reg(uint8_t reg, uint8_t value) {
    const uint8_t frame[2] = {reg, value};
    return bus_.write(rgb_addr_, frame, 2);
}

bool Lcd1602Rgb::init() {
    const uint8_t function = kFunctionSet | kTwoLine;

    sleep_ms(50);  // the controller needs time after power-on

    // Sent four times with these delays, exactly as DFRobot's library does.
    // The controller does not reliably latch the first attempt.
    if (!command(function)) return false;
    sleep_us(4500);
    command(function);
    sleep_us(150);
    command(function);
    command(function);

    command(kDisplayControl | kDisplayOn);  // display on, no cursor, no blink
    clear();
    command(kEntryModeSet | kEntryLeft);

    // Backlight: wake the PCA9633 (it powers up asleep), put all four channels
    // under individual PWM control, then default to white.
    set_reg(kRegMode1, 0x00);
    set_reg(kRegLedOut, 0xFF);
    set_reg(kRegMode2, 0x20);
    set_backlight(255, 255, 255);

    return true;
}

void Lcd1602Rgb::clear() {
    command(kClearDisplay);
    sleep_ms(2);  // clear is slow and does not report busy over I2C
}

void Lcd1602Rgb::set_cursor(uint8_t col, uint8_t row) {
    // Row 0 starts at DDRAM 0x00, row 1 at 0x40; bit 7 makes it a set-address
    // command.
    const uint8_t addr = static_cast<uint8_t>(row == 0 ? (col | 0x80) : (col | 0xC0));
    command(addr);
}

void Lcd1602Rgb::print(const char *text) {
    // One character per transfer, matching DFRobot's library. Batching them
    // behind a single 0x40 control byte would be faster, but a 16-character
    // line still costs only a few milliseconds and this is known to work.
    for (const char *p = text; *p; ++p) {
        write_char(static_cast<uint8_t>(*p));
    }
}

void Lcd1602Rgb::print_at(uint8_t col, uint8_t row, const char *text) {
    set_cursor(col, row);
    print(text);
}

void Lcd1602Rgb::print_line(uint8_t row, const char *text) {
    set_cursor(0, row);
    size_t len = strlen(text);
    for (uint8_t i = 0; i < kCols; i++) {
        write_char(i < len ? static_cast<uint8_t>(text[i]) : ' ');
    }
}

void Lcd1602Rgb::set_backlight(uint8_t r, uint8_t g, uint8_t b) {
    set_reg(kRegRed, r);
    set_reg(kRegGreen, g);
    set_reg(kRegBlue, b);
}

void Lcd1602Rgb::set_backlight_hue(float hue_deg) {
    while (hue_deg < 0.0f) hue_deg += 360.0f;
    while (hue_deg >= 360.0f) hue_deg -= 360.0f;

    const int   sector = static_cast<int>(hue_deg / 60.0f) % 6;
    const float frac = (hue_deg / 60.0f) - static_cast<float>(static_cast<int>(hue_deg / 60.0f));
    const uint8_t rising = static_cast<uint8_t>(frac * 255.0f);
    const uint8_t falling = static_cast<uint8_t>((1.0f - frac) * 255.0f);

    switch (sector) {
        case 0: set_backlight(255, rising, 0); break;
        case 1: set_backlight(falling, 255, 0); break;
        case 2: set_backlight(0, 255, rising); break;
        case 3: set_backlight(0, falling, 255); break;
        case 4: set_backlight(rising, 0, 255); break;
        default: set_backlight(255, 0, falling); break;
    }
}

}  // namespace drivers
