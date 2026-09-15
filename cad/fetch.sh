#!/usr/bin/env bash
# Fetch third-party CAD models for enclosure design.
#
# The .step files themselves are gitignored — they are other people's binaries
# and re-fetchable. This script is what gets committed, so the set is
# reproducible.

set -euo pipefail
cd "$(dirname "$0")"

ADA="https://raw.githubusercontent.com/adafruit/Adafruit_CAD_Parts/main"
SFE="https://raw.githubusercontent.com/sparkfun/3D_Models/master/products"

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
get "$ADA/3013%20DS3231%20RTC/3013%20DS3231%20RTC.step" "ds3231-rtc-3013.step"

# The 63.5 mm wide protoboard. SparkFun publish an STL, not a STEP, and the
# board is a plain rectangle with no mounting holes -- so the dimensions in
# README.md are more use than the 4 MB mesh.
get "$SFE/12070/stl/SIK-DIP-board.stl" "sparkfun-solderable-breadboard-12070.stl"

if [ -f pico-w.zip ]; then unzip -o -q pico-w.zip -d pico-w && rm pico-w.zip; fi
echo
ls -la

# ---------------------------------------------------------------------------
# Bare-part models for the fabricated board.
#
# KiCad's 3D library — free, no login, real STEP solids. These are standard
# packages, so the models are dimensionally exact for our parts.
KIC="https://gitlab.com/kicad/libraries/kicad-packages3D/-/raw/master"

get "$KIC/Connector_JST.3dshapes/JST_XH_B2B-XH-A_1x02_P2.50mm_Vertical.step" "jst-xh-b2b.step"
get "$KIC/Connector_JST.3dshapes/JST_XH_B4B-XH-A_1x04_P2.50mm_Vertical.step" "jst-xh-b4b.step"
get "$KIC/Connector_JST.3dshapes/JST_XH_B5B-XH-A_1x05_P2.50mm_Vertical.step" "jst-xh-b5b.step"
get "$KIC/Connector_PinSocket_2.54mm.3dshapes/PinSocket_1x20_P2.54mm_Vertical.step" "pinsocket-1x20.step"
get "$KIC/Connector_PinHeader_2.54mm.3dshapes/PinHeader_1x20_P2.54mm_Vertical.step" "pinheader-1x20.step"
get "$KIC/Package_SO.3dshapes/SOIC-16W_7.5x10.3mm_P1.27mm.step"              "soic-16w-ds3231.step"
get "$KIC/Package_SO.3dshapes/SOIC-14_3.9x8.7mm_P1.27mm.step"                "soic-14-sn74hct125.step"
get "$KIC/Package_TO_SOT_SMD.3dshapes/SOT-23.step"                           "sot-23.step"

# LCSC / EasyEDA — the manufacturers' own geometry for the parts JLCPCB stock.
# These are OBJ meshes, not STEP solids, but they are the exact parts. The 3D
# model UUID is resolved from the LCSC code at fetch time so this stays valid
# if EasyEDA re-hosts them.
mkdir -p lcsc
lcsc() {  # lcsc <LCSC code> <output name>
    printf '%-34s ' "lcsc/$2.obj"
    uuid=$(curl -fsSL --max-time 30 \
        "https://easyeda.com/api/products/$1/components?version=6.4.19.5" \
        | python3 -c "
import json,sys
for s in (json.load(sys.stdin).get('result') or {}).get('packageDetail',{}).get('dataStr',{}).get('shape',[]):
    if s.startswith('SVGNODE'):
        print(json.loads(s.split('~',1)[1]).get('attrs',{}).get('uuid','')); break
" 2>/dev/null)
    if [ -z "$uuid" ]; then printf 'no 3D model\n'; return; fi
    if curl -fsSL --max-time 60 "https://modules.easyeda.com/3dmodel/$uuid" -o "lcsc/$2.obj"; then
        printf 'ok  %s\n' "$(du -h "lcsc/$2.obj" | cut -f1)"
    else
        printf 'FAILED\n'; rm -f "lcsc/$2.obj"
    fi
}

lcsc C3659294  scd41              # 10.10 x 10.10 x  6.32 mm
lcsc C3659348  mics-5524          #  7.00 x  5.00 x  1.52 mm
lcsc C389589   pir-bs612          #  9.85 x  9.10 x  9.20 mm  (incl. 6 mm leads)
lcsc C504893   veml7700           #  6.80 x  2.35 x  3.00 mm
lcsc C161819   buzzer-qmb-09b-05  #  9.00 x  9.00 x 10.01 mm  (body 5.5 above board)
lcsc C70381    cr1220-holder      # 19.76 x 14.90 x  4.90 mm
