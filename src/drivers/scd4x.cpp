#include "drivers/scd4x.hpp"

#include "drivers/sensirion.hpp"

namespace drivers {
namespace {

// Commands, each with the execution time the datasheet requires before its
// reply may be read. Getting those delays too short is the usual cause of a
// driver that works "most of the time".
constexpr uint16_t kStartPeriodic   = 0x21B1;  // 0 ms
constexpr uint16_t kReadMeasurement = 0xEC05;  // 1 ms
constexpr uint16_t kStopPeriodic    = 0x3F86;  // 500 ms
constexpr uint16_t kGetDataReady    = 0xE4B8;  // 1 ms
constexpr uint16_t kGetSerial       = 0x3682;  // 1 ms
constexpr uint16_t kSetTempOffset   = 0x241D;  // 1 ms
constexpr uint16_t kSetAltitude     = 0x2427;  // 1 ms

// The SCD4x scales by 2^16, unlike the SHT3x which uses 2^16 - 1. A small
// difference, but copying the SHT formula across would bias every reading.
constexpr float kScale = 65536.0f;

}  // namespace

Scd4x::Scd4x(SoftI2C &bus, uint8_t addr) : bus_(bus), addr_(addr) {}

bool Scd4x::send_command(uint16_t cmd, uint32_t exec_ms) {
    uint8_t frame[2];
    sensirion::command_bytes(cmd, frame);
    if (!bus_.write(addr_, frame, 2)) return false;
    if (exec_ms) sleep_ms(exec_ms);
    return true;
}

bool Scd4x::send_command_arg(uint16_t cmd, uint16_t arg, uint32_t exec_ms) {
    uint8_t frame[5];
    sensirion::command_bytes(cmd, frame);
    frame[2] = static_cast<uint8_t>(arg >> 8);
    frame[3] = static_cast<uint8_t>(arg & 0xFF);
    frame[4] = sensirion::crc(&frame[2], 2);  // CRC covers the argument only
    if (!bus_.write(addr_, frame, 5)) return false;
    if (exec_ms) sleep_ms(exec_ms);
    return true;
}

bool Scd4x::read_words(uint16_t cmd, uint32_t exec_ms, uint16_t *words, size_t count) {
    if (count == 0 || count > 3) return false;
    if (!send_command(cmd, exec_ms)) return false;

    uint8_t rx[9];
    const size_t bytes = count * 3;
    if (!bus_.read(addr_, rx, bytes)) return false;

    for (size_t i = 0; i < count; i++) {
        if (!sensirion::read_word(&rx[i * 3], words[i])) return false;
    }
    return true;
}

bool Scd4x::stop_periodic() { return send_command(kStopPeriodic, 500); }
bool Scd4x::start_periodic() { return send_command(kStartPeriodic, 1); }

bool Scd4x::begin() {
    // A previous run may have left periodic mode going, and most commands are
    // rejected while it is. Stopping first costs 500 ms once at boot and makes
    // start-up deterministic. A failure here is expected on a bus with no
    // sensor, so it is not treated as fatal on its own.
    stop_periodic();

    uint16_t words[3];
    if (!read_words(kGetSerial, 1, words, 3)) return false;

    serial_ = (static_cast<uint64_t>(words[0]) << 32) | (static_cast<uint64_t>(words[1]) << 16) |
              static_cast<uint64_t>(words[2]);

    if (!start_periodic()) return false;

    // Nothing to read for five seconds; poll() reports not-ready until then.
    next_poll_ = make_timeout_time_ms(kPollIntervalMs);
    return true;
}

bool Scd4x::data_ready(bool &ready) {
    uint16_t word = 0;
    if (!read_words(kGetDataReady, 1, &word, 1)) return false;
    // Only the low 11 bits carry the answer; the rest are undefined and must
    // be masked off rather than compared against zero wholesale.
    ready = (word & 0x07FF) != 0;
    return true;
}

bool Scd4x::read_measurement() {
    uint16_t words[3];
    if (!read_words(kReadMeasurement, 1, words, 3)) return false;

    co2_ppm_ = words[0];
    temperature_c_ = -45.0f + 175.0f * (static_cast<float>(words[1]) / kScale);
    humidity_pct_ = 100.0f * (static_cast<float>(words[2]) / kScale);

    if (humidity_pct_ < 0.0f) humidity_pct_ = 0.0f;
    if (humidity_pct_ > 100.0f) humidity_pct_ = 100.0f;

    has_reading_ = true;
    return true;
}

bool Scd4x::poll() {
    if (!time_reached(next_poll_)) return false;
    next_poll_ = make_timeout_time_ms(kPollIntervalMs);

    bool ready = false;
    if (!data_ready(ready) || !ready) return false;
    return read_measurement();
}

bool Scd4x::set_temperature_offset(float celsius) {
    if (celsius < 0.0f) celsius = 0.0f;
    const uint16_t word = static_cast<uint16_t>(celsius * kScale / 175.0f + 0.5f);
    return send_command_arg(kSetTempOffset, word, 1);
}

bool Scd4x::set_altitude(uint16_t metres) { return send_command_arg(kSetAltitude, metres, 1); }

}  // namespace drivers
