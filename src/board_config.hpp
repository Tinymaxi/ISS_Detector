// Pin assignments for Die ISS-Box.
//
// Target: Raspberry Pi Pico WH (RP2040). Physical pin numbers are given for
// breadboard work — they are what you count on the board, not the GPIO number.

#pragma once

#include "pico/stdlib.h"

namespace board {

// --- NeoPixels: two independent chains, one data pin each ------------------
// Separate rather than chained. The ring animates at 25 fps while the digits
// change once a second, so chaining would mean pushing 56 pixels every frame
// to update 24 of them. It also keeps the breadboard wiring simpler and stops
// a fault in one display taking out the other.
constexpr uint PIN_RING    = 2;       // physical 4
constexpr uint RING_PIXELS = 24;

// Four digits of eight pixels: offsets 0-6 are segments a..g in order, offset
// 7 is that digit's dot. Layout taken from the Siebensegment Uhr project.
constexpr uint PIN_DIGITS        = 3;  // physical 5
constexpr uint DIGIT_COUNT       = 4;
constexpr uint PIXELS_PER_DIGIT  = 8;
constexpr uint DIGIT_PIXELS      = DIGIT_COUNT * PIXELS_PER_DIGIT;  // 32

// --- I2C: SHT3x/4x + DFR0464 LCD share one software-driven bus -------------
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
