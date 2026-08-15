// Pin assignments for Die ISS-Box.
//
// Target: Raspberry Pi Pico WH (RP2040). Physical pin numbers are given for
// breadboard work — they are what you count on the board, not the GPIO number.

#pragma once

#include "hardware/i2c.h"
#include "pico/stdlib.h"

namespace board {

// --- NeoPixels -------------------------------------------------------------
// The 24-pixel ring and the 32-pixel seven-segment are chained on one data
// line: ring DOUT -> seg DIN. Ring pixels come first in the frame buffer.
constexpr uint PIN_NEOPIXEL   = 2;    // physical 4
constexpr uint NEOPIXEL_RING  = 24;
constexpr uint NEOPIXEL_SEG   = 32;
constexpr uint NEOPIXEL_TOTAL = NEOPIXEL_RING + NEOPIXEL_SEG;

// --- I2C0: SHT3x/4x + DFR0464 LCD share the bus ----------------------------
inline i2c_inst_t *const I2C_PORT = i2c0;
constexpr uint PIN_I2C_SDA  = 4;      // physical 6
constexpr uint PIN_I2C_SCL  = 5;      // physical 7
constexpr uint I2C_BAUDRATE = 100'000;

// --- Controls --------------------------------------------------------------
constexpr uint PIN_ENCODER_SW = 13;   // physical 17, internal pull-up
constexpr uint PIN_ENCODER_A  = 14;   // physical 19
constexpr uint PIN_ENCODER_B  = 15;   // physical 20
constexpr uint PIN_BUTTON     = 16;   // physical 21, internal pull-up to GND
constexpr uint PIN_BUTTON_LAMP= 17;   // physical 22, via transistor
constexpr uint PIN_BUZZER     = 18;   // physical 24, PWM

// --- MiCS-5524 gas sensor --------------------------------------------------
// EN is inverted: driving it LOW enables the heater. A0 arrives through a
// 10k/15k divider because the sensor runs at 5 V and the ADC tolerates 3.3 V.
constexpr uint PIN_GAS_EN     = 19;   // physical 25
constexpr uint PIN_GAS_AO     = 26;   // physical 31, ADC0
constexpr uint GAS_ADC_CHANNEL = 0;

}  // namespace board
