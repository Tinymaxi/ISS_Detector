// Stage 2 bring-up: find out what is actually on the I2C bus.
//
// Resolves two open questions in one run — which Sensirion part is fitted
// (SHT3x vs SHT4x behave differently), and the DFR0464 LCD's two addresses,
// which differ between hardware revisions.

#pragma once

namespace tools {

// Configures I2C0 on the pins in board_config.hpp. Safe to call once at boot.
void i2c_bus_init();

// Scans 0x08..0x77, naming any address this project expects, and actively
// probes Sensirion addresses to say which sensor is present.
void i2c_scan();

}  // namespace tools
