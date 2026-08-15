// Bit-banged I2C master.
//
// Why not the RP2040's hardware I2C block? During Stage 2 bring-up the
// peripheral would not talk to the DFR0464 LCD at any bus speed from 400 kHz
// down to 10 kHz, reporting a timeout with no abort source — meaning it never
// started a transfer. The same wires, at the same moment, worked perfectly
// when driven in software. The root cause was never confirmed.
//
// Rather than keep digging, this project drives I2C manually. The cost is
// negligible: the bus carries an LCD refreshed a few times a second and a
// temperature sensor read once a second, nowhere near needing DMA or hardware
// timing. The benefit is a bus that demonstrably works and whose every edge is
// under our control — including recovery from a slave left holding SDA low,
// which matters for a toy whose power gets yanked mid-transfer.

#pragma once

#include <cstddef>
#include <cstdint>

#include "pico/stdlib.h"

namespace drivers {

class SoftI2C {
public:
    // bit_delay_us sets roughly a quarter of the clock period; 5 µs gives
    // something in the region of 100 kHz once overheads are counted.
    SoftI2C(uint sda_pin, uint scl_pin, uint bit_delay_us = 5);

    // Configures both pins as open-drain-with-pull-up and frees the bus if a
    // device is holding it. Call once before any transfer.
    void init();

    // Clocks SCL until a stuck slave releases SDA, then issues a STOP.
    // Returns 0 if the bus was already free, the pulse count needed to free
    // it, or -1 if SDA is still low after nine pulses.
    int recover();

    // Address-only transaction. True if a device acknowledged.
    bool probe(uint8_t addr);

    // True only if the address and every data byte were acknowledged.
    bool write(uint8_t addr, const uint8_t *data, size_t len, bool send_stop = true);

    // Reads len bytes, ACKing all but the last. True if the address was
    // acknowledged and the read completed.
    bool read(uint8_t addr, uint8_t *data, size_t len);

    // Convenience for the very common "write a register, read it back" shape.
    bool write_then_read(uint8_t addr, const uint8_t *tx, size_t tx_len, uint8_t *rx,
                         size_t rx_len);

private:
    void release(uint pin);
    void drive_low(uint pin);
    void setup_pin(uint pin);
    bool scl_high();            // honours clock stretching
    void start();
    void stop();
    void write_bit(bool bit);
    bool read_bit();
    bool write_byte(uint8_t byte);   // returns the ACK
    uint8_t read_byte(bool ack);

    uint sda_;
    uint scl_;
    uint delay_us_;
};

}  // namespace drivers
