#include "drivers/i2c_bus.hpp"

#include "board_config.hpp"

namespace drivers {

SoftI2C &i2c_bus() {
    // Function-local static: constructed on first use, so there is no
    // dependence on static initialisation order across translation units.
    static SoftI2C bus(board::PIN_I2C_SDA, board::PIN_I2C_SCL);
    return bus;
}

}  // namespace drivers
