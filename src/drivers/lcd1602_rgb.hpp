// DFRobot Gravity LCD1602 RGB (DFR0464 v1.0).
//
// Two I2C devices in one module:
//   0x3E  AiP31068 character controller — HD44780-style command set, but
//         reached over I2C with a leading control byte (0x80 = command,
//         0x40 = data) rather than parallel RS/E lines.
//   0x60  PCA9633 4-channel LED driver, running the RGB backlight.
//
// The initialisation sequence follows DFRobot's own library, including its
// oddities: the function-set command is sent four times with specific delays,
// which is what the controller wants after power-on.

#pragma once

#include <cstdint>

#include "drivers/soft_i2c.hpp"

namespace drivers {

class Lcd1602Rgb {
public:
    static constexpr uint8_t kDefaultLcdAddr = 0x3E;
    static constexpr uint8_t kDefaultRgbAddr = 0x60;
    static constexpr uint8_t kCols = 16;
    static constexpr uint8_t kRows = 2;

    Lcd1602Rgb(SoftI2C &bus, uint8_t lcd_addr = kDefaultLcdAddr,
               uint8_t rgb_addr = kDefaultRgbAddr);

    // Returns false if the character controller never acknowledged.
    bool init();

    void clear();
    void set_cursor(uint8_t col, uint8_t row);
    void print(const char *text);
    void print_at(uint8_t col, uint8_t row, const char *text);

    // Writes exactly kCols characters, padding with spaces, so a shorter line
    // erases whatever was there before without a full clear() flicker.
    void print_line(uint8_t row, const char *text);

    void set_backlight(uint8_t r, uint8_t g, uint8_t b);

    // Hue in degrees [0,360), full saturation and value. Convenient for the
    // mode-colour and gas-score displays later on.
    void set_backlight_hue(float hue_deg);

private:
    bool command(uint8_t value);
    bool write_char(uint8_t value);
    bool set_reg(uint8_t reg, uint8_t value);

    SoftI2C &bus_;
    uint8_t  lcd_addr_;
    uint8_t  rgb_addr_;
};

}  // namespace drivers
