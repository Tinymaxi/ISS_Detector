// MiCS-5524 metal-oxide gas sensor (Adafruit breakout).
//
// Adafruit are explicit that this part cannot tell gases apart — it responds
// to CO, ammonia, ethanol, hydrogen and hydrocarbons all at once, and is only
// good for measuring *change* in a known gas. That single fact shapes the
// whole design: everything here is relative to a rolling baseline, never an
// absolute reading. Conveniently, hydrogen and methane are exactly the funny
// ones.
//
// The sensor runs at 5 V and its A0 output swings to 5 V, so it reaches the
// 3.3 V ADC through a resistor divider. The divider ratio is passed in rather
// than assumed, because it depends on which resistors were to hand.
//
// EN drives Q2's gate directly, so HIGH enables the heater and we hold it high.
// The Adafruit breakout was the opposite — it had its own switching transistor
// behind EN — so this flipped when the design moved to the bare sensor.

#pragma once

#include <cstdint>

#include "pico/stdlib.h"

namespace drivers {

class GasSensor {
public:
    // r_top sits between A0 and the ADC pin, r_bottom between the ADC pin and
    // ground. Values in ohms; only their ratio matters.
    GasSensor(uint adc_pin, uint adc_channel, uint en_pin, float r_top = 10000.0f,
              float r_bottom = 4700.0f);

    void init();

    // The heater draws 25-35 mA continuously and needs about a second after
    // being switched on before readings mean anything.
    void set_heater(bool on);

    // Averages several conversions; the raw ADC is noisy at this impedance.
    uint16_t read_raw();

    // Voltage at the sensor's own A0 pin, with the divider undone.
    float read_sensor_volts();

    // Call regularly. Tracks a slowly-moving baseline of "room air", so the
    // score measures a change rather than an absolute level, and so the
    // sensor's drift over hours quietly cancels out.
    void update();

    float latest_volts() const { return latest_v_; }
    float baseline_volts() const { return baseline_v_; }

    // How far above baseline we currently are, in volts. Negative clamps to 0.
    float excess_volts() const;

    // Freezes the baseline — call before a measurement so the thing being
    // measured cannot creep into the reference it is measured against.
    void hold_baseline(bool held) { baseline_held_ = held; }

    // Snaps the baseline to the current reading, for a fresh start.
    void reset_baseline();

    // 0-99, mapped generously: a device that tells a proud five-year-old "0"
    // is a broken toy. full_scale_volts is the excess that reads as 99.
    int score(float excess_v, float full_scale_volts = 0.35f) const;

private:
    uint  adc_pin_;
    uint  adc_channel_;
    uint  en_pin_;
    float divider_scale_;  // multiply the ADC voltage by this to get A0

    float latest_v_ = 0.0f;
    float baseline_v_ = 0.0f;
    bool  baseline_ready_ = false;
    bool  baseline_held_ = false;
};

}  // namespace drivers
