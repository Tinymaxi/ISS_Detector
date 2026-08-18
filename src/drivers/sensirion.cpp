#include "drivers/sensirion.hpp"

namespace drivers {
namespace sensirion {

uint8_t crc(const uint8_t *data, size_t len) {
    uint8_t c = 0xFF;
    for (size_t i = 0; i < len; i++) {
        c ^= data[i];
        for (int bit = 0; bit < 8; bit++) {
            c = (c & 0x80) ? static_cast<uint8_t>((c << 1) ^ 0x31) : static_cast<uint8_t>(c << 1);
        }
    }
    return c;
}

bool word_ok(const uint8_t *triple) { return crc(triple, 2) == triple[2]; }

bool read_word(const uint8_t *triple, uint16_t &out) {
    if (!word_ok(triple)) return false;
    out = static_cast<uint16_t>((triple[0] << 8) | triple[1]);
    return true;
}

}  // namespace sensirion
}  // namespace drivers
