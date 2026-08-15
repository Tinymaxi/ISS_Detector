// Stage 2 bring-up: find out what is actually on the I2C bus.
//
// Resolves two open questions in one run — which Sensirion part is fitted
// (SHT3x vs SHT4x behave differently), and the DFR0464 LCD's two addresses,
// which differ between hardware revisions.

#pragma once

namespace tools {

// Configures I2C0 on the pins in board_config.hpp. Safe to call once at boot.
void i2c_bus_init();

// Electrical check before any protocol runs: are SDA and SCL actually being
// held high by a module's pull-up resistors? Answers "is anything wired to
// these pins at all", which a failed address scan cannot distinguish from
// "wired, but not answering". Leaves the bus configured for I2C afterwards.
void i2c_check_bus_lines();

// Scans 0x08..0x77, naming any address this project expects, and actively
// probes Sensirion addresses to say which sensor is present.
//
// Probes each address twice: once with a read and once with a one-byte write.
// Some controllers — the LCD's AiP31068 among them — are write-only and never
// acknowledge a read, so a read-only scan would miss them entirely.
void i2c_scan();

}  // namespace tools
