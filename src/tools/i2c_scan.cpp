#include "tools/i2c_scan.hpp"

#include <cstdio>

#include "board_config.hpp"
#include "hardware/i2c.h"
#include "pico/stdlib.h"

namespace tools {
namespace {

// Addresses this build actually cares about. The DFR0464 LCD is two devices in
// one module (character controller + backlight driver) and their addresses vary
// by hardware revision, which is exactly why we scan rather than assume.
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
    {0x62, "LCD RGB backlight (PCA9633 alternate)"},
    {0x6B, "LCD RGB backlight (some DFR0464 revisions)"},
    {0x70, "PCA9633 All-Call — an echo of the backlight driver, not a real device"},
};

const char *describe(uint8_t addr) {
    for (const auto &k : kKnown) {
        if (k.addr == addr) return k.what;
    }
    return nullptr;
}

// Sensirion CRC-8: polynomial 0x31, initialised to 0xFF.
uint8_t sensirion_crc(const uint8_t *data, size_t len) {
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; bit++) {
            crc = (crc & 0x80) ? static_cast<uint8_t>((crc << 1) ^ 0x31)
                               : static_cast<uint8_t>(crc << 1);
        }
    }
    return crc;
}

// SHT4x answers command 0x89 with a 6-byte serial number. SHT3x does not know
// that command, so a clean CRC-checked reply is a positive identification.
bool looks_like_sht4x(uint8_t addr, uint32_t &serial_out) {
    const uint8_t cmd = 0x89;
    if (i2c_write_timeout_us(board::I2C_PORT, addr, &cmd, 1, false, 10000) != 1) {
        return false;
    }
    sleep_ms(10);

    uint8_t rx[6];
    if (i2c_read_timeout_us(board::I2C_PORT, addr, rx, sizeof(rx), false, 10000) !=
        static_cast<int>(sizeof(rx))) {
        return false;
    }
    if (sensirion_crc(&rx[0], 2) != rx[2] || sensirion_crc(&rx[3], 2) != rx[5]) {
        return false;
    }

    serial_out = (static_cast<uint32_t>(rx[0]) << 24) | (static_cast<uint32_t>(rx[1]) << 16) |
                 (static_cast<uint32_t>(rx[3]) << 8) | rx[4];
    return true;
}

// SHT3x answers the 16-bit command 0xF32D with a 3-byte status register.
bool looks_like_sht3x(uint8_t addr, uint16_t &status_out) {
    const uint8_t cmd[2] = {0xF3, 0x2D};
    if (i2c_write_timeout_us(board::I2C_PORT, addr, cmd, 2, false, 10000) != 2) {
        return false;
    }
    sleep_ms(10);

    uint8_t rx[3];
    if (i2c_read_timeout_us(board::I2C_PORT, addr, rx, sizeof(rx), false, 10000) !=
        static_cast<int>(sizeof(rx))) {
        return false;
    }
    if (sensirion_crc(&rx[0], 2) != rx[2]) {
        return false;
    }

    status_out = static_cast<uint16_t>((rx[0] << 8) | rx[1]);
    return true;
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

void i2c_bus_init() {
    i2c_init(board::I2C_PORT, board::I2C_BAUDRATE);
    gpio_set_function(board::PIN_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(board::PIN_I2C_SCL, GPIO_FUNC_I2C);
    // The breakouts carry their own pull-ups; these are a harmless safety net
    // that also keeps the bus sane if a module is unplugged mid-run.
    gpio_pull_up(board::PIN_I2C_SDA);
    gpio_pull_up(board::PIN_I2C_SCL);
}

void i2c_scan() {
    printf("\n--- I2C scan on GP%u/GP%u (SDA/SCL) ---\n", board::PIN_I2C_SDA,
           board::PIN_I2C_SCL);

    int found = 0;
    // 0x00-0x07 and 0x78-0x7F are reserved by the I2C spec.
    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        uint8_t dummy;
        int ret = i2c_read_timeout_us(board::I2C_PORT, addr, &dummy, 1, false, 10000);
        if (ret < 0) continue;

        found++;
        const char *what = describe(addr);
        printf("  0x%02X  %s\n", addr, what ? what : "unknown device");

        if (addr == 0x44 || addr == 0x45 || addr == 0x46) {
            identify_sensirion(addr);
        }
    }

    if (found == 0) {
        printf("  nothing found.\n");
        printf("  check: 3V3 (pin 36) and GND to both modules, SDA->GP4 (pin 6),\n");
        printf("         SCL->GP5 (pin 7). SDA/SCL swapped is the usual culprit.\n");
    } else {
        printf("  %d device%s total.\n", found, found == 1 ? "" : "s");
    }
}

}  // namespace tools
