#include "drivers/buzzer.hpp"

#include <cmath>

#include "hardware/clocks.h"
#include "hardware/pwm.h"

namespace drivers {
namespace {

constexpr uint32_t NOTE_C4 = 261, NOTE_D4 = 293, NOTE_E4 = 329, NOTE_G4 = 392;
constexpr uint32_t NOTE_C5 = 523, NOTE_E5 = 659, NOTE_G5 = 784, NOTE_A5 = 880;
constexpr uint32_t NOTE_C6 = 1046;

// Each note sounds for this fraction of its slot, the remainder being silence.
// Without it, two identical notes in a row merge into one long tone.
constexpr float kArticulation = 0.85f;

inline float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }

}  // namespace

Buzzer::Buzzer(uint gpio) : gpio_(gpio) {}

void Buzzer::init() {
    gpio_set_function(gpio_, GPIO_FUNC_PWM);
    slice_ = pwm_gpio_to_slice_num(gpio_);
    channel_ = pwm_gpio_to_channel(gpio_);

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, 1.0f);
    pwm_init(slice_, &cfg, true);
    pwm_set_gpio_level(gpio_, 0);
}

void Buzzer::start_tone(uint32_t freq_hz) {
    if (freq_hz == 0) {
        stop_tone();
        return;
    }

    const double clk = static_cast<double>(clock_get_hz(clk_sys));

    // Pick the smallest divider that still lets TOP fit in 16 bits.
    double div = ceil(clk / (static_cast<double>(freq_hz) * 65536.0));
    if (div < 1.0) div = 1.0;
    if (div > 255.0) div = 255.0;

    double top = (clk / (div * static_cast<double>(freq_hz))) - 1.0;

    // If TOP came out very small the duty resolution would be coarse, so trade
    // some divider range for a bigger TOP.
    if (top < 200.0 && div < 255.0) {
        const double better = ceil(clk / (static_cast<double>(freq_hz) * 400.0));
        if (better >= 1.0 && better <= 255.0) {
            div = better;
            top = (clk / (div * static_cast<double>(freq_hz))) - 1.0;
        }
    }

    if (top > 65535.0) top = 65535.0;
    if (top < 1.0) top = 1.0;

    const uint16_t wrap = static_cast<uint16_t>(llround(top));
    pwm_set_clkdiv(slice_, static_cast<float>(div));
    pwm_set_wrap(slice_, wrap);
    // 50% duty scaled by volume gives the loudest square wave at volume 1.
    pwm_set_chan_level(slice_, channel_,
                       static_cast<uint16_t>((wrap + 1u) * 0.5f * volume_));
    pwm_set_enabled(slice_, true);
    tone_on_ = true;
}

void Buzzer::stop_tone() {
    pwm_set_chan_level(slice_, channel_, 0);
    tone_on_ = false;
}

void Buzzer::begin_note(size_t index) {
    const Note &n = melody_[index];
    const uint32_t slot_us = static_cast<uint32_t>(n.duration_ms) * 1000u;
    note_end_ = make_timeout_time_us(slot_us);
    tone_end_ = make_timeout_time_us(static_cast<uint32_t>(slot_us * kArticulation));
    start_tone(n.freq_hz);
}

void Buzzer::play(const Note *melody, size_t length, float volume) {
    if (melody == nullptr || length == 0) {
        stop();
        return;
    }
    volume_ = clamp01(volume);
    melody_ = melody;
    length_ = length;
    index_ = 0;
    begin_note(0);
}

void Buzzer::beep(uint16_t freq_hz, uint16_t duration_ms, float volume) {
    scratch_ = {freq_hz, duration_ms};
    play(&scratch_, 1, volume);
}

void Buzzer::stop() {
    stop_tone();
    melody_ = nullptr;
    length_ = 0;
    index_ = 0;
}

void Buzzer::update() {
    if (melody_ == nullptr) return;

    if (tone_on_ && time_reached(tone_end_)) {
        stop_tone();  // brief gap so the next note is distinct
    }

    if (!time_reached(note_end_)) return;

    index_++;
    if (index_ >= length_) {
        stop();
        return;
    }
    begin_note(index_);
}

// --- melodies --------------------------------------------------------------

const Buzzer::Note *Buzzer::startup(size_t &length) {
    static constexpr Note kMelody[] = {
        {NOTE_C5, 90}, {NOTE_E5, 90}, {NOTE_G5, 90}, {NOTE_C6, 180},
    };
    length = sizeof(kMelody) / sizeof(kMelody[0]);
    return kMelody;
}

const Buzzer::Note *Buzzer::click(size_t &length) {
    static constexpr Note kMelody[] = {{NOTE_A5, 25}};
    length = sizeof(kMelody) / sizeof(kMelody[0]);
    return kMelody;
}

const Buzzer::Note *Buzzer::iss_alert(size_t &length) {
    // Deliberately nagging: a two-tone siren that is hard to ignore from
    // another room, because the whole point is to send a child outdoors.
    static constexpr Note kMelody[] = {
        {NOTE_G5, 160}, {NOTE_C6, 160}, {NOTE_G5, 160}, {NOTE_C6, 160},
        {NOTE_G5, 160}, {NOTE_C6, 160}, {0, 200},
        {NOTE_G5, 160}, {NOTE_C6, 160}, {NOTE_G5, 160}, {NOTE_C6, 320},
    };
    length = sizeof(kMelody) / sizeof(kMelody[0]);
    return kMelody;
}

const Buzzer::Note *Buzzer::fanfare(size_t &length) {
    static constexpr Note kMelody[] = {
        {NOTE_C5, 120}, {NOTE_E5, 120}, {NOTE_G5, 120},
        {NOTE_C6, 240}, {0, 60},        {NOTE_G5, 120}, {NOTE_C6, 400},
    };
    length = sizeof(kMelody) / sizeof(kMelody[0]);
    return kMelody;
}

const Buzzer::Note *Buzzer::raspberry(size_t &length) {
    // A descending slide, the musical equivalent of a shrug.
    static constexpr Note kMelody[] = {
        {NOTE_G4, 140}, {NOTE_E4, 140}, {NOTE_D4, 140}, {NOTE_C4, 420},
    };
    length = sizeof(kMelody) / sizeof(kMelody[0]);
    return kMelody;
}

}  // namespace drivers
