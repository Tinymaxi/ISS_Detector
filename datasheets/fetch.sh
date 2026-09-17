#!/usr/bin/env bash
# Fetch the manufacturer datasheets for the parts in this build.
#
# Unlike cad/fetch.sh, the PDFs here ARE committed. Datasheet URLs rot -- vendors
# reorganise, parts go end-of-life, distributors vanish -- and the point of this
# folder is that the numbers are still here in five years. This script records
# where each one came from and re-fetches anything lost.
#
# Existing files are skipped. Run with FORCE=1 to re-download everything.
#
# Not fetched here, because no stable public URL exists:
#   arcade-button-AB30L5-B.pdf   supplier PDF from BerryBase
#   rgbdigit.pdf                 vendor PDF, downloaded by hand
#   buzzer-UDB-05B.pdf           mirrored from akizukidenshi (see README)

set -euo pipefail
cd "$(dirname "$0")"

UA='Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0 Safari/537.36'
ok=0; skip=0; fail=0

get() {  # get <output> <url> [referer]
    printf '%-26s ' "$1"
    if [ -f "$1" ] && [ "${FORCE:-0}" != "1" ]; then
        printf 'have  %s\n' "$(du -h "$1" | cut -f1)"; skip=$((skip+1)); return
    fi
    if curl -fsSL --max-time 120 -A "$UA" -e "${3:-}" "$2" -o "$1.part"; then
        # A vendor 404 page served as HTML is not a datasheet.
        if head -c 5 "$1.part" | grep -q '%PDF'; then
            mv "$1.part" "$1"; printf 'ok    %s\n' "$(du -h "$1" | cut -f1)"; ok=$((ok+1)); return
        fi
        rm -f "$1.part"; printf 'NOT A PDF\n'
    else
        rm -f "$1.part"; printf 'FAILED\n'
    fi
    fail=$((fail+1))
}

# --- microcontroller -------------------------------------------------------
get "rp2040.pdf"   "https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf"
get "pico-w.pdf"   "https://datasheets.raspberrypi.com/picow/pico-w-datasheet.pdf"

# --- I2C sensors -----------------------------------------------------------
get "scd4x.pdf"    "https://sensirion.com/media/documents/48C4B7FB/67FE0194/CD_DS_SCD4x_Datasheet_D1.pdf"
get "sht3x.pdf"    "https://sensirion.com/media/documents/213E6A3B/63A5A569/Datasheet_SHT3x_DIS.pdf"

# Vishay serve their own PDFs: 84286 is the datasheet, 84323 the application
# note with the optical window and cover-glass guidance the datasheet omits.
get "veml7700.pdf"          "https://www.vishay.com/docs/84286/veml7700.pdf"          "https://www.vishay.com/"
get "veml7700-design-in.pdf" "https://www.vishay.com/docs/84323/designingveml7700.pdf" "https://www.vishay.com/"

# Maxim/ADI block automated requests; Adafruit mirror it under the 3013 product.
get "ds3231.pdf"   "https://cdn-shop.adafruit.com/product-files/3013/DS3231.pdf"

# --- pixels ----------------------------------------------------------------
get "ws2812b.pdf"  "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf"

# --- analog and discrete ---------------------------------------------------
get "mics-5524.pdf"  "https://cdn-shop.adafruit.com/product-files/3199/MiCS-5524.pdf"
get "sn74hct125.pdf" "https://www.ti.com/lit/ds/symlink/sn74hct125.pdf"

# Three vendors refuse automated requests (onsemi and NXP answer 403/404 to
# anything that is not a browser session), so these come from mirrors. Check
# the revision against the vendor page if a number ever looks wrong.
get "pca9633-backlight.pdf" "https://images.100y.com.tw/pdf_file/35-NXP-PCA9633x.pdf" "https://images.100y.com.tw/"
get "2n2222a.pdf"           "https://web.mit.edu/6.101/www/reference/2N2222A.pdf"      "https://web.mit.edu/"
get "1n5817-schottky.pdf"   "https://www.vishay.com/docs/88525/1n5817.pdf"             "https://www.vishay.com/"

echo
echo "$ok fetched, $skip already present, $fail failed."
