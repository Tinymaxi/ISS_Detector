#include "tools/i2c_scan.hpp"

#include <cstdio>

#include "drivers/i2c_bus.hpp"
#include "drivers/sensirion.hpp"

namespace tools {
namespace {

// Addresses this build cares about. The DFR0464 LCD is two devices in one
// module (character controller + backlight driver) and their addresses vary by
// hardware revision, which is why we scan rather than assume.
struct KnownAddress {
    uint8_t     addr;
    const char *what;
};

constexpr KnownAddress kKnown[] = {
    {0x27, "LCD character controller (AiP31068) — or a PCF8574 backpack"},
    {0x2D, "LCD RGB backlight (SN3193, DFR0464 v2.0)"},
    {0x3E, "LCD character controller (AiP31068, DFR0464 v1.0)"},
    {0x44, "SHT3x/SHT4x temp+humidity (default address)"},
    {0x45, "SHT3x alternate / SHT4x variant"},
    {0x46, "SHT4x variant"},
    {0x60, "LCD RGB backlight (PCA9633, DFR0464 v1.0)"},
    {0x62, "SCD40/SCD41 CO2+temp+humidity — collides with a PCA9633 alternate"},
    {0x68, "DS3231 precision RTC (fixed address)"},
    {0x6B, "LCD RGB backlight (some DFR0464 revisions)"},
    {0x70, "PCA9633 All-Call — an echo of the backlight driver, not a real device"},
};

const char *describe(uint8_t addr) {
    for (const auto &k : kKnown) {
        if (k.addr == addr) return k.what;
    }
    return nullptr;
}

// SHT4x answers command 0x89 with a 6-byte serial number; SHT3x does not know
// that command. A clean CRC-checked reply is a positive identification.
bool looks_like_sht4x(uint8_t addr, uint32_t &serial_out) {
    auto &bus = drivers::i2c_bus();
    const uint8_t cmd = 0x89;
    if (!bus.write(addr, &cmd, 1)) return false;
    sleep_ms(10);

    uint8_t rx[6];
    if (!bus.read(addr, rx, sizeof(rx))) return false;
    if (!drivers::sensirion::word_ok(&rx[0]) || !drivers::sensirion::word_ok(&rx[3])) {
        return false;
    }

    serial_out = (static_cast<uint32_t>(rx[0]) << 24) | (static_cast<uint32_t>(rx[1]) << 16) |
                 (static_cast<uint32_t>(rx[3]) << 8) | rx[4];
    return true;
}

// SHT3x answers the 16-bit command 0xF32D with a 3-byte status register.
bool looks_like_sht3x(uint8_t addr, uint16_t &status_out) {
    auto &bus = drivers::i2c_bus();
    const uint8_t cmd[2] = {0xF3, 0x2D};
    if (!bus.write(addr, cmd, 2)) return false;
    sleep_ms(10);

    uint8_t rx[3];
    if (!bus.read(addr, rx, sizeof(rx))) return false;
    if (!drivers::sensirion::word_ok(&rx[0])) return false;

    status_out = static_cast<uint16_t>((rx[0] << 8) | rx[1]);
    return true;
}

// SCD4x answers get_serial_number (0x3682) with three CRC'd words. It only
// does so when not mid-measurement, so a failure here is inconclusive rather
// than proof of absence — a previous boot may have left periodic mode running.
bool looks_like_scd4x(uint8_t addr, uint64_t &serial_out) {
    auto &bus = drivers::i2c_bus();
    const uint8_t cmd[2] = {0x36, 0x82};
    if (!bus.write(addr, cmd, 2)) return false;
    sleep_ms(2);

    uint8_t rx[9];
    if (!bus.read(addr, rx, sizeof(rx))) return false;

    uint16_t w[3];
    for (int i = 0; i < 3; i++) {
        if (!drivers::sensirion::read_word(&rx[i * 3], w[i])) return false;
    }
    serial_out = (static_cast<uint64_t>(w[0]) << 32) | (static_cast<uint64_t>(w[1]) << 16) | w[2];
    return true;
}

void identify_scd4x(uint8_t addr) {
    uint64_t serial = 0;
    if (looks_like_scd4x(addr, serial)) {
        printf("            -> SCD4x confirmed, serial 0x%012llX\n",
               (unsigned long long)serial);
    } else {
        printf("            -> no serial; either mid-measurement, or this is a\n");
        printf("               PCA9633 backlight rather than an SCD4x\n");
    }
}

void identify_sensirion(uint8_t addr) {
    uint32_t serial = 0;
    if (looks_like_sht4x(addr, serial)) {
        printf("            -> SHT4x confirmed, serial 0x%08lX\n", (unsigned long)serial);
        return;
    }

    uint16_t status = 0;
    if (looks_like_sht3x(addr, status)) {
        printf("            -> SHT3x confirmed, status 0x%04X\n", status);
        return;
    }

    printf("            -> responds, but is neither SHT3x nor SHT4x\n");
}

}  // namespace

void i2c_scan() {
    auto &bus = drivers::i2c_bus();

    int freed = bus.recover();
    if (freed > 0) {
        printf("[i2c] bus was stuck; freed after %d clock pulses\n", freed);
    } else if (freed < 0) {
        printf("[i2c] bus STILL STUCK — a device is holding SDA low\n");
    }

    printf("\n--- I2C scan ---\n");

    int found = 0;
    // 0x00-0x07 and 0x78-0x7F are reserved by the I2C spec.
    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        if (!bus.probe(addr)) continue;

        found++;
        const char *what = describe(addr);
        printf("  0x%02X  %s\n", addr, what ? what : "unknown device");

        if (addr == 0x44 || addr == 0x45 || addr == 0x46) {
            identify_sensirion(addr);
        } else if (addr == 0x62) {
            identify_scd4x(addr);
        }
    }

    if (found == 0) {
        printf("  nothing found.\n");
    } else {
        printf("  %d device%s total.\n", found, found == 1 ? "" : "s");
    }
}

}  // namespace tools
