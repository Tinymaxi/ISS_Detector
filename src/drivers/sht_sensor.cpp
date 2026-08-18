#include "drivers/sht_sensor.hpp"

#include "drivers/sensirion.hpp"
#include "pico/stdlib.h"

namespace drivers {
namespace {

// SHT4x
constexpr uint8_t kSht4xReadSerial = 0x89;
constexpr uint8_t kSht4xMeasureHigh = 0xFD;  // high repeatability, no heater

// SHT3x — 16-bit commands
constexpr uint8_t kSht3xReadStatus[2] = {0xF3, 0x2D};
constexpr uint8_t kSht3xMeasureHigh[2] = {0x24, 0x00};  // clock stretching off

}  // namespace

ShtSensor::ShtSensor(SoftI2C &bus, uint8_t addr) : bus_(bus), addr_(addr) {}

const char *ShtSensor::kind_name() const {
    switch (kind_) {
        case Kind::Sht3x: return "SHT3x";
        case Kind::Sht4x: return "SHT4x";
        default: return "unknown";
    }
}

bool ShtSensor::init() {
    kind_ = Kind::Unknown;
    id_ = 0;

    // Try SHT4x first: its serial-number command is a single byte, which an
    // SHT3x will not recognise.
    if (bus_.write(addr_, &kSht4xReadSerial, 1)) {
        sleep_ms(10);
        uint8_t rx[6];
        if (bus_.read(addr_, rx, sizeof(rx)) && sensirion::crc(&rx[0], 2) == rx[2] &&
            sensirion::crc(&rx[3], 2) == rx[5]) {
            kind_ = Kind::Sht4x;
            id_ = (static_cast<uint32_t>(rx[0]) << 24) | (static_cast<uint32_t>(rx[1]) << 16) |
                  (static_cast<uint32_t>(rx[3]) << 8) | rx[4];
            return true;
        }
    }

    if (bus_.write(addr_, kSht3xReadStatus, 2)) {
        sleep_ms(10);
        uint8_t rx[3];
        if (bus_.read(addr_, rx, sizeof(rx)) && sensirion::crc(&rx[0], 2) == rx[2]) {
            kind_ = Kind::Sht3x;
            id_ = static_cast<uint32_t>((rx[0] << 8) | rx[1]);
            return true;
        }
    }

    return false;
}

bool ShtSensor::read_sht4x() {
    if (!bus_.write(addr_, &kSht4xMeasureHigh, 1)) return false;
    sleep_ms(10);  // high repeatability needs ~8.3 ms

    uint8_t rx[6];
    if (!bus_.read(addr_, rx, sizeof(rx))) return false;
    if (sensirion::crc(&rx[0], 2) != rx[2] || sensirion::crc(&rx[3], 2) != rx[5]) return false;

    const uint16_t raw_t = static_cast<uint16_t>((rx[0] << 8) | rx[1]);
    const uint16_t raw_h = static_cast<uint16_t>((rx[3] << 8) | rx[4]);

    temperature_c_ = -45.0f + 175.0f * (static_cast<float>(raw_t) / 65535.0f);
    humidity_pct_ = -6.0f + 125.0f * (static_cast<float>(raw_h) / 65535.0f);

    // The SHT4x conversion can produce slightly out-of-range humidity at the
    // extremes; the datasheet says to clip.
    if (humidity_pct_ < 0.0f) humidity_pct_ = 0.0f;
    if (humidity_pct_ > 100.0f) humidity_pct_ = 100.0f;
    return true;
}

bool ShtSensor::read_sht3x() {
    if (!bus_.write(addr_, kSht3xMeasureHigh, 2)) return false;
    sleep_ms(20);  // high repeatability needs ~15 ms

    uint8_t rx[6];
    if (!bus_.read(addr_, rx, sizeof(rx))) return false;
    if (sensirion::crc(&rx[0], 2) != rx[2] || sensirion::crc(&rx[3], 2) != rx[5]) return false;

    const uint16_t raw_t = static_cast<uint16_t>((rx[0] << 8) | rx[1]);
    const uint16_t raw_h = static_cast<uint16_t>((rx[3] << 8) | rx[4]);

    temperature_c_ = -45.0f + 175.0f * (static_cast<float>(raw_t) / 65535.0f);
    humidity_pct_ = 100.0f * (static_cast<float>(raw_h) / 65535.0f);
    return true;
}

bool ShtSensor::read() {
    switch (kind_) {
        case Kind::Sht4x: return read_sht4x();
        case Kind::Sht3x: return read_sht3x();
        default: return false;
    }
}

}  // namespace drivers
