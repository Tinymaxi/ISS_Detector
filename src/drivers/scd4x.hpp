// Sensirion SCD40 / SCD41 — CO2, temperature and humidity over I2C at 0x62.
//
// Not a drop-in for the SHT3x it replaces: different address, different command
// set, and a fundamentally different rhythm. The SCD4x runs a photoacoustic CO2
// measurement that takes five seconds and pulses to around 205 mA while it
// works, so it is put into periodic mode once and then polled for readiness
// rather than being asked for a reading on demand.
//
// Three consequences worth knowing:
//
//   Nothing is available for the first five seconds after start-up. begin()
//   does not wait for it; poll() simply returns false until the sensor is ready.
//
//   The 205 mA pulses land on the same 3.3 V regulator as the LCD, which is
//   good for about 300 mA. Worth measuring rather than assuming.
//
//   Those same pulses warm the sensor, so its temperature reads high by a
//   couple of degrees. That is what the temperature offset is for — and it is
//   why a separate SHT3x is still the better source for room temperature if you
//   keep one fitted.

#pragma once

#include <cstdint>

#include "drivers/soft_i2c.hpp"
#include "pico/stdlib.h"

namespace drivers {

class Scd4x {
public:
    static constexpr uint8_t kDefaultAddr = 0x62;

    // The sensor produces a new sample every 5 s, so polling faster than this
    // only wastes bus time.
    static constexpr uint32_t kPollIntervalMs = 1000;

    explicit Scd4x(SoftI2C &bus, uint8_t addr = kDefaultAddr);

    // Stops any measurement left running from a previous boot, reads the serial
    // number to confirm the part is really there, then starts periodic mode.
    // False means nothing answered, or it answered with a bad CRC.
    bool begin();

    // Call every time round the main loop. Returns true on the loop where a
    // fresh sample landed. Rate-limits itself internally.
    bool poll();

    uint16_t co2_ppm() const { return co2_ppm_; }
    float    temperature_c() const { return temperature_c_; }
    float    humidity_pct() const { return humidity_pct_; }

    // 48-bit unique id, read during begin().
    uint64_t serial() const { return serial_; }

    // True once a first valid sample has arrived.
    bool has_reading() const { return has_reading_; }

    // Compensates the self-heating described above. Persisted in the sensor
    // until changed; the factory default is 4 °C.
    bool set_temperature_offset(float celsius);

    // Improves accuracy by telling the sensor its altitude in metres. Also
    // persisted. Bern is about 540 m.
    bool set_altitude(uint16_t metres);

    bool stop_periodic();
    bool start_periodic();

private:
    bool send_command(uint16_t cmd, uint32_t exec_ms);
    bool send_command_arg(uint16_t cmd, uint16_t arg, uint32_t exec_ms);
    bool read_words(uint16_t cmd, uint32_t exec_ms, uint16_t *words, size_t count);
    bool data_ready(bool &ready);
    bool read_measurement();

    SoftI2C &bus_;
    uint8_t  addr_;

    uint64_t serial_ = 0;
    uint16_t co2_ppm_ = 0;
    float    temperature_c_ = 0.0f;
    float    humidity_pct_ = 0.0f;
    bool     has_reading_ = false;

    absolute_time_t next_poll_{};
};

}  // namespace drivers
