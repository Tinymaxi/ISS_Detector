// Die ISS-Box — Stage 4: the LCD says hello
//
// Scans the bus once, then drives the display: text on both lines, an uptime
// counter proving it refreshes, and a backlight slowly working through the
// colour wheel.

#include <cstdio>

#include "board_config.hpp"
#include "drivers/i2c_bus.hpp"
#include "drivers/lcd1602_rgb.hpp"
#include "hardware/clocks.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
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

int main() {
    stdio_init_all();
    wait_for_console();

    if (!led_init()) {
        while (true) {
            printf("cyw43_arch_init() failed\n");
            sleep_ms(1000);
        }
    }

    printf("\n=== ISS-Box Stage 4 ===\n");
    printf("board: %s   sdk: %s\n", PICO_BOARD, PICO_SDK_VERSION_STRING);

    drivers::i2c_bus().init();
    tools::i2c_scan();

    drivers::Lcd1602Rgb lcd(drivers::i2c_bus());
    const bool lcd_ok = lcd.init();
    if (!lcd_ok) {
        printf("LCD did not acknowledge at 0x%02X\n", drivers::Lcd1602Rgb::kDefaultLcdAddr);
    } else {
        printf("LCD up.\n");
    }

    lcd.print_line(0, "Die ISS-Box");
    lcd.print_line(1, "Hallo!");
    sleep_ms(2000);

    float    hue = 0.0f;
    uint32_t seconds = 0;
    while (true) {
        char line[17];
        snprintf(line, sizeof(line), "laeuft seit %lus", (unsigned long)seconds);
        lcd.print_line(1, line);

        // A full turn of the colour wheel every 30 s.
        lcd.set_backlight_hue(hue);
        hue += 12.0f;

        // Heartbeat on the console too, so a hung I2C transfer is obvious.
        printf("t=%lus  hue=%3.0f  lcd=%s\n", (unsigned long)seconds, (double)hue,
               lcd_ok ? "ok" : "FAILED");

        led_set(true);
        sleep_ms(100);
        led_set(false);
        sleep_ms(900);
        seconds++;
    }
}
