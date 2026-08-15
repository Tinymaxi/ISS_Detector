// Stage 2 bring-up: find out what is actually on the I2C bus.
//
// Resolves two open questions in one run — which Sensirion part is fitted
// (SHT3x and SHT4x share address 0x44 but speak different commands), and the
// DFR0464 LCD's two addresses, which differ between hardware revisions.

#pragma once

namespace tools {

// Scans 0x08..0x77 over the shared software I2C bus, naming any address this
// project expects, and actively interrogating Sensirion addresses to say which
// sensor is present. Recovers the bus first if a device is holding SDA low.
void i2c_scan();

}  // namespace tools
