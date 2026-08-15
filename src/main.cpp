// Die ISS-Box — Stage 2: I2C bus scan
//
// Rescans every few seconds, so modules can be plugged and unplugged while
// watching the console. The onboard LED keeps blinking as a liveness signal.

#include <cstdio>

#include "board_config.hpp"
#include "hardware/clocks.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "drivers/i2c_bus.hpp"
#include "tools/i2c_scan.hpp"

// On Pico W the onboard LED is driven by the wireless chip, not by a GPIO,
// so cyw43_arch has to be up before the LED will do anything.
static bool led_init() {
    return cyw43_arch_init() == 0;
}

static void led_set(bool on) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
}

// Give the host a few seconds to open the USB serial port, but never block
// forever — the finished toy runs with nothing attached.
static void wait_for_console(uint32_t timeout_ms = 3000) {
    absolute_time_t deadline = make_timeout_time_ms(timeout_ms);
    while (!stdio_usb_connected() && !time_reached(deadline)) {
        sleep_ms(50);
    }
}

// Blink for the given time so the board visibly stays alive between scans.
static void blink_for(uint32_t total_ms) {
    absolute_time_t deadline = make_timeout_time_ms(total_ms);
    while (!time_reached(deadline)) {
        led_set(true);
        sleep_ms(100);
        led_set(false);
        sleep_ms(900);
    }
}

int main() {
    stdio_init_all();
    wait_for_console();

    if (!led_init()) {
        // Nothing else can be trusted if the wireless chip did not come up.
        while (true) {
            printf("cyw43_arch_init() failed\n");
            sleep_ms(1000);
        }
    }

    printf("\n=== ISS-Box Stage 2 ===\n");
    printf("board: %s   sdk: %s\n", PICO_BOARD, PICO_SDK_VERSION_STRING);
    printf("sys clock: %u kHz\n", clock_get_hz(clk_sys) / 1000);

    drivers::i2c_bus().init();

    while (true) {
        tools::i2c_scan();
        blink_for(5000);
    }
}
