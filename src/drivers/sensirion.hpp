// Bits shared by every Sensirion sensor on this bus.
//
// Both the SHT3x and the SCD4x wrap their data in the same CRC-8 and both send
// 16-bit words big-endian, so this lives in one place rather than being copied
// into each driver.

#pragma once

#include <cstddef>
#include <cstdint>

namespace drivers {
namespace sensirion {

// CRC-8, polynomial 0x31, initialised to 0xFF. Covers two data bytes at a time.
uint8_t crc(const uint8_t *data, size_t len);

// Checks one word+CRC triple as it arrives on the wire.
bool word_ok(const uint8_t *triple);

// Decodes a word+CRC triple. Returns false on a CRC mismatch.
bool read_word(const uint8_t *triple, uint16_t &out);

// Splits a 16-bit command into the two bytes the sensor expects.
inline void command_bytes(uint16_t cmd, uint8_t out[2]) {
    out[0] = static_cast<uint8_t>(cmd >> 8);
    out[1] = static_cast<uint8_t>(cmd & 0xFF);
}

}  // namespace sensirion
}  // namespace drivers
