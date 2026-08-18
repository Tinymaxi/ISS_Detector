#!/usr/bin/env bash
# Fetch third-party CAD models for enclosure design.
#
# The .step files themselves are gitignored — they are other people's binaries
# and re-fetchable. This script is what gets committed, so the set is
# reproducible.

set -euo pipefail
cd "$(dirname "$0")"

ADA="https://raw.githubusercontent.com/adafruit/Adafruit_CAD_Parts/main"

get() {  # get <url> <output>
    printf '%-34s ' "$2"
    if curl -fsSL --max-time 60 "$1" -o "$2"; then
        printf 'ok  %s\n' "$(du -h "$2" | cut -f1)"
    else
        printf 'FAILED\n'
    fi
}

get "https://datasheets.raspberrypi.com/picow/PicoW-step.zip" "pico-w.zip"
get "$ADA/1586%20NeoPixel%20Ring%2024x/24x%20NeoPixel%20Ring%201586.step" "neopixel-ring-24.step"
get "$ADA/5187%20SCD-40%20C02%20Sensor/5187%20SCD-40%20C02%20Sensor.step" "scd40.step"
get "$ADA/1609%20Perma-Proto%20HalfSize/1609%20Perma-Proto%20HalfSize.step" "permaproto-half.step"
get "$ADA/1608%20perma-proto%20quarter/1608%20perma-proto%20quarter.step" "permaproto-quarter.step"
get "$ADA/723%20Minty%20Perma-Proto/723%20Minty%20Perma-Proto.step" "minty-permaproto.step"
get "$ADA/471%20Arcade%20Button%2030mm/471%20Arcade%20Button%2030mm.step" "arcade-button-30mm.step"
get "$ADA/4090%20USB%20C%20Breakout/4090%20USB%20C%20Breakout.step" "usb-c-breakout-4090.step"

if [ -f pico-w.zip ]; then unzip -o -q pico-w.zip -d pico-w && rm pico-w.zip; fi
echo
ls -la
