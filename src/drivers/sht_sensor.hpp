// Sensirion SHT3x / SHT4x temperature and humidity sensor.
//
// Both families answer at 0x44, but they speak different commands, so the part
// is identified at init rather than configured by hand:
//
//   SHT4x answers 0x89 with a six-byte serial number.
//   SHT3x answers 0xF32D with a three-byte status word.
//
// Whichever replies with a valid CRC wins. Every reply is CRC-checked — these
// sensors are on the same bus as the LCD, and a corrupted reading that looks
// plausible is worse than no reading at all.

#pragma once

#include <cstdint>

#include "drivers/soft_i2c.hpp"

namespace drivers {

class ShtSensor {
public:
    enum class Kind { Unknown, Sht3x, Sht4x };

    static constexpr uint8_t kDefaultAddr = 0x44;

    explicit ShtSensor(SoftI2C &bus, uint8_t addr = kDefaultAddr);

    // Identifies the part. False means nothing usable answered at the address.
    bool init();

    Kind        kind() const { return kind_; }
    const char *kind_name() const;

    // Serial number for SHT4x, status word for SHT3x. Zero if unknown.
    uint32_t id() const { return id_; }

    // Takes a high-repeatability measurement. False on a bus error or a failed
    // CRC, in which case the previous values are left untouched.
    bool read();

    float temperature_c() const { return temperature_c_; }
    float humidity_pct() const { return humidity_pct_; }

private:
    bool read_sht4x();
    bool read_sht3x();

    SoftI2C &bus_;
    uint8_t  addr_;
    Kind     kind_ = Kind::Unknown;
    uint32_t id_ = 0;
    float    temperature_c_ = 0.0f;
    float    humidity_pct_ = 0.0f;
};

// Sensirion CRC-8: polynomial 0x31, initialised to 0xFF. Shared with the bus
// scanner, which uses it for the same identification trick.
uint8_t sensirion_crc(const uint8_t *data, size_t len);

}  // namespace drivers
