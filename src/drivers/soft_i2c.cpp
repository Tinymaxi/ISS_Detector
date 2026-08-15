#include "drivers/soft_i2c.hpp"

#include "hardware/gpio.h"

namespace drivers {

SoftI2C::SoftI2C(uint sda_pin, uint scl_pin, uint bit_delay_us)
    : sda_(sda_pin), scl_(scl_pin), delay_us_(bit_delay_us) {}

// Open-drain is emulated the usual way: "drive low" switches the pin to an
// output that already holds 0, "release" switches it back to an input and lets
// the pull-up raise it. The output register is never set to 1, so we never
// fight another device for the line.
void SoftI2C::release(uint pin) { gpio_set_dir(pin, GPIO_IN); }
void SoftI2C::drive_low(uint pin) { gpio_set_dir(pin, GPIO_OUT); }

void SoftI2C::setup_pin(uint pin) {
    gpio_init(pin);
    gpio_put(pin, 0);  // the value driven whenever the pin becomes an output
    gpio_pull_up(pin);
    release(pin);
}

void SoftI2C::init() {
    setup_pin(sda_);
    setup_pin(scl_);
    sleep_us(20);
    recover();
}

bool SoftI2C::scl_high() {
    release(scl_);
    // A slave may hold the clock down to buy itself time. Wait, but not
    // forever — a permanently low clock is a fault, not back-pressure.
    for (int i = 0; i < 1000; i++) {
        if (gpio_get(scl_)) return true;
        sleep_us(1);
    }
    return false;
}

int SoftI2C::recover() {
    if (gpio_get(sda_)) return 0;

    // Clock the stuck slave through the rest of its byte. Nine pulses covers
    // eight data bits plus the ACK slot.
    int pulses = 0;
    while (pulses < 9 && !gpio_get(sda_)) {
        drive_low(scl_);
        sleep_us(delay_us_);
        release(scl_);
        sleep_us(delay_us_);
        pulses++;
    }
    stop();
    return gpio_get(sda_) ? pulses : -1;
}

void SoftI2C::start() {
    release(sda_);
    scl_high();
    sleep_us(delay_us_);
    drive_low(sda_);
    sleep_us(delay_us_);
    drive_low(scl_);
    sleep_us(delay_us_);
}

void SoftI2C::stop() {
    drive_low(sda_);
    sleep_us(delay_us_);
    scl_high();
    sleep_us(delay_us_);
    release(sda_);
    sleep_us(delay_us_);
}

void SoftI2C::write_bit(bool bit) {
    if (bit) {
        release(sda_);
    } else {
        drive_low(sda_);
    }
    sleep_us(delay_us_);
    scl_high();
    sleep_us(delay_us_);
    drive_low(scl_);
    sleep_us(delay_us_);
}

bool SoftI2C::read_bit() {
    release(sda_);
    sleep_us(delay_us_);
    scl_high();
    sleep_us(delay_us_);
    bool bit = gpio_get(sda_);
    drive_low(scl_);
    sleep_us(delay_us_);
    return bit;
}

bool SoftI2C::write_byte(uint8_t byte) {
    for (int i = 7; i >= 0; i--) {
        write_bit((byte >> i) & 1);
    }
    return !read_bit();  // the slave pulls SDA low to acknowledge
}

uint8_t SoftI2C::read_byte(bool ack) {
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        byte = static_cast<uint8_t>((byte << 1) | (read_bit() ? 1 : 0));
    }
    write_bit(!ack);  // drive SDA low to ACK, leave it high to NACK
    return byte;
}

bool SoftI2C::probe(uint8_t addr) {
    start();
    bool acked = write_byte(static_cast<uint8_t>(addr << 1));
    stop();
    return acked;
}

bool SoftI2C::write(uint8_t addr, const uint8_t *data, size_t len, bool send_stop) {
    start();
    if (!write_byte(static_cast<uint8_t>(addr << 1))) {
        stop();
        return false;
    }
    for (size_t i = 0; i < len; i++) {
        if (!write_byte(data[i])) {
            stop();
            return false;
        }
    }
    if (send_stop) stop();
    return true;
}

bool SoftI2C::read(uint8_t addr, uint8_t *data, size_t len) {
    start();
    if (!write_byte(static_cast<uint8_t>((addr << 1) | 1))) {
        stop();
        return false;
    }
    for (size_t i = 0; i < len; i++) {
        data[i] = read_byte(i + 1 < len);  // NACK the final byte
    }
    stop();
    return true;
}

bool SoftI2C::write_then_read(uint8_t addr, const uint8_t *tx, size_t tx_len, uint8_t *rx,
                              size_t rx_len) {
    // A STOP between the two halves rather than a repeated START. Both the
    // Sensirion sensors and the LCD are happy with it, and it keeps the state
    // machine simple.
    if (!write(addr, tx, tx_len, true)) return false;
    return read(addr, rx, rx_len);
}

}  // namespace drivers
