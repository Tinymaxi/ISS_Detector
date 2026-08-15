// The one I2C bus in this build, shared by the SHT sensor and the LCD.

#pragma once

#include "drivers/soft_i2c.hpp"

namespace drivers {

// Returns the shared bus. Call init() on it once at boot before any transfer.
SoftI2C &i2c_bus();

}  // namespace drivers
