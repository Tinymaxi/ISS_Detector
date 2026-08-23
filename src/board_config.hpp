// Pin assignments for Die ISS-Box.
//
// Target: Raspberry Pi Pico WH (RP2040). Physical pin numbers are given
// throughout — they are what you count on the board, not the GPIO number.
//
// LAID OUT FOR HAND-SOLDERING. The breadboard is temporary; the real build
// goes onto Adafruit Perma-Proto boards, where adjacent pins
// carrying unrelated wires invite solder bridges and leave no room to dress
// the wiring. So: at least one free pin between signals from *different*
// modules. Pins from the same module stay adjacent on purpose — they arrive
// as one bundle and are easier to solder as one.
//
//   pin  4  GP2   ring data
//   pin  5  GP3   .
//   pin  6  GP4   I2C SDA   ─┐ same bundle
//   pin  7  GP5   I2C SCL   ─┘
//   pin  8  GND
//   pin  9  GP6   .
//   pin 10  GP7   digit data
//   pin 11  GP8   .
//   pin 12  GP9   buzzer
//   pin 13  GND
//   pin 14  GP10  .
//   pin 15  GP11  RTC SQW (1 Hz tick)
//   pin 16  GP12  .
//   pin 17  GP13  encoder SW   ─┐
//   pin 18  GND   encoder GND   │ same bundle
//   pin 19  GP14  encoder CLK   │
//   pin 20  GP15  encoder DT   ─┘
//   pin 21  GP16  arcade button ─┐ same bundle
//   pin 22  GP17  arcade lamp   ─┘
//   pin 31  GP26  gas A0   ─┐ same bundle
//   pin 32  GP27  gas EN   ─┘
//   pin 33  AGND  analog ground, right next to the gas divider
//
// Rails: 3V3 = pin 36, VBUS (5 V) = pin 40, GND = 3/8/13/18/23/28/38.

#pragma once

#include "pico/stdlib.h"

namespace board {

// --- NeoPixels: two independent chains, one data pin each ------------------
// Separate rather than chained. The ring animates at 25 fps while the digits
// change once a second, so chaining would mean pushing 56 pixels every frame
// to update 24 of them. It also isolates a fault in one display from the other.
constexpr uint PIN_RING    = 2;       // physical 4
constexpr uint RING_PIXELS = 24;

// Four digits of eight pixels: offsets 0-6 are segments a..g in order, offset
// 7 is that digit's dot. Layout taken from the Siebensegment Uhr project.
constexpr uint PIN_DIGITS        = 7;  // physical 10
constexpr uint DIGIT_COUNT       = 4;
constexpr uint PIXELS_PER_DIGIT  = 8;
constexpr uint DIGIT_PIXELS      = DIGIT_COUNT * PIXELS_PER_DIGIT;  // 32

// How many digit modules are actually soldered up right now. Chaining fewer
// than DIGIT_COUNT is fine — the modules present consume their own data and
// pass the remainder downstream to nothing. Raise this to DIGIT_COUNT once all
// four are wired.
constexpr uint DIGITS_FITTED        = 1;
constexpr uint DIGIT_PIXELS_FITTED  = DIGITS_FITTED * PIXELS_PER_DIGIT;

// --- I2C: LCD (0x3E + 0x60), SCD40 (0x62) and DS3231 RTC (0x68) ------------
// One software-driven bus, four addresses, no collisions. Every device on it
// runs at 3V3 -- each carries its own pull-ups to whatever feeds it, and the
// RP2040 is not 5 V tolerant.
constexpr uint PIN_I2C_SDA  = 4;      // physical 6
constexpr uint PIN_I2C_SCL  = 5;      // physical 7

// --- DS3231 precision RTC (Adafruit 3013) ----------------------------------
// The clock the box actually keeps time by. NTP needs WiFi association before
// it can answer, which is seconds of blank display on every power-up; the
// DS3231 has the time on the bus immediately, and holds it on a CR1220 through
// a yanked plug. NTP still runs, but to discipline the RTC rather than to be
// the clock. At +/-2 ppm it drifts about two minutes a year unaided.
//
// SQW is a 1 Hz square wave, open-drain, so it needs a pull-up -- use the
// RP2040's internal one, no resistor. Strictly optional: the time can just be
// read over I2C. It is wired anyway because adding a wire to a sealed box is
// much harder than fitting one now.
constexpr uint PIN_RTC_SQW = 11;      // physical 15, internal pull-up
constexpr uint8_t RTC_ADDR = 0x68;

// --- Rotary encoder (KY-040 style: CLK, DT, SW, +, GND) --------------------
// Power it from 3V3, never 5 V: the module's onboard pull-ups sit on CLK and
// DT, so a 5 V supply would put 5 V straight onto these pins, and the RP2040
// is not 5 V tolerant.
//
// CLK and DT must stay consecutive with CLK first — the PIO quadrature decoder
// reads them as a two-bit field from a base pin.
constexpr uint PIN_ENCODER_SW  = 13;  // physical 17, needs an internal pull-up
constexpr uint PIN_ENCODER_CLK = 14;  // physical 19, the PIO base pin
constexpr uint PIN_ENCODER_DT  = 15;  // physical 20

// --- Arcade button ---------------------------------------------------------
constexpr uint PIN_BUTTON      = 16;  // physical 21, internal pull-up, to GND
constexpr uint PIN_BUTTON_LAMP = 17;  // physical 22, via transistor

// --- Buzzer ----------------------------------------------------------------
constexpr uint PIN_BUZZER = 9;        // physical 12, PWM

// --- MiCS-5524 gas sensor --------------------------------------------------
// EN is inverted: driving it LOW enables the heater. A0 arrives through a
// 10k/15k divider because the sensor runs at 5 V and the ADC tolerates 3.3 V.
constexpr uint PIN_GAS_AO      = 26;  // physical 31, ADC0
constexpr uint GAS_ADC_CHANNEL = 0;
constexpr uint PIN_GAS_EN      = 27;  // physical 32

}  // namespace board
