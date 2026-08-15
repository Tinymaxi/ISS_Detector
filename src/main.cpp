// Die ISS-Box — Stage 1: toolchain check (blink + printf over USB)
//
// Verifies the build works end to end before any peripheral is wired:
//   - the onboard LED blinks
//   - printf reaches the host over USB CDC

#include <cstdio>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/clocks.h"

// On Pico 2 W the onboard LED is driven by the wireless chip, not by a GPIO,
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

    printf("\n=== ISS-Box Stage 1 ===\n");
    printf("board: %s   sdk: %s\n", PICO_BOARD, PICO_SDK_VERSION_STRING);
    printf("sys clock: %u kHz\n", clock_get_hz(clk_sys) / 1000);

    uint32_t ticks = 0;
    while (true) {
        led_set(true);
        sleep_ms(100);
        led_set(false);
        sleep_ms(900);

        printf("alive %lu (%.1f s)\n",
               (unsigned long)++ticks,
               to_ms_since_boot(get_absolute_time()) / 1000.0f);
    }
}
