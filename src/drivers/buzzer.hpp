// Passive buzzer on a PWM pin.
//
// The tone generation is lifted from the Buzzer project in ~/pico/Buzzer —
// picking a clock divider and wrap value that land on the requested frequency
// is the fiddly part and that code is proven.
//
// The sequencing is not. That version played melodies by sleeping between
// notes, which would freeze the ring mid-animation. Here a melody is a state
// machine advanced by update(), so the box keeps drawing while it sings — the
// ISS alarm in particular has to sound *while* the compass keeps updating.
//
// Melodies are static arrays rather than std::vector, so playing one costs no
// allocation.

#pragma once

#include <cstddef>
#include <cstdint>

#include "pico/stdlib.h"

namespace drivers {

class Buzzer {
public:
    struct Note {
        uint16_t freq_hz;      // 0 = rest
        uint16_t duration_ms;
    };

    explicit Buzzer(uint gpio);

    void init();

    // Call every time round the main loop.
    void update();

    bool is_playing() const { return melody_ != nullptr; }

    // Starts a melody, replacing anything already playing. The array must
    // outlive playback — use the static melodies below or a constexpr array.
    void play(const Note *melody, size_t length, float volume = 0.6f);

    void beep(uint16_t freq_hz = 1600, uint16_t duration_ms = 60, float volume = 0.6f);
    void stop();

    // --- built-in melodies -------------------------------------------------
    static const Note *startup(size_t &length);
    static const Note *click(size_t &length);
    static const Note *iss_alert(size_t &length);   // urgent, for a pass
    static const Note *fanfare(size_t &length);     // a good score
    static const Note *raspberry(size_t &length);   // a bad one

private:
    void start_tone(uint32_t freq_hz);
    void stop_tone();
    void begin_note(size_t index);

    uint  gpio_;
    uint  slice_ = 0;
    uint  channel_ = 0;
    float volume_ = 0.6f;

    const Note *melody_ = nullptr;
    size_t      length_ = 0;
    size_t      index_ = 0;

    // Each note stops slightly before the next begins, so repeated notes are
    // heard as separate rather than one long tone.
    absolute_time_t tone_end_{};
    absolute_time_t note_end_{};
    bool            tone_on_ = false;

    // A one-shot beep is just a one-note melody, stored here so callers do not
    // have to keep an array alive.
    Note scratch_{0, 0};
};

}  // namespace drivers
