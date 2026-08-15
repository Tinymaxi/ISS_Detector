# Die ISS-Box

A birthday present for a five-year-old: one box, four modes, driven by a big arcade button
and a knob.

| Mode | NeoPixel ring (24 px) | NeoPixel 7-segment | LCD |
|---|---|---|---|
| **UHR** | 24-hour dial | `HH:MM`, NTP-synced | date + weekday |
| **ISS** | compass pointing at the space station | distance in km | ground position |
| **WETTER** | thermometer arc, blue → red | temperature | temp + humidity |
| **PUPS** | countdown, then a 0–24 bar graph | score 0–99 | verdict |

The ring is the centrepiece: 24 pixels divide 360° into exactly 15° each, so with a north arrow
marked on the case, the lit pixel genuinely points at the ISS. Colour carries elevation — deep blue
when it's on the far side of the planet, blazing white when it's overhead.

Whatever mode the box is in, it interrupts itself when the ISS actually rises above the horizon:
buzzer, ring flash, and an automatic switch to ISS mode.

## Hardware

Raspberry Pi Pico WH (RP2040), C++17 on the Pico SDK. The "H" means headers are pre-soldered,
including the three debug pins — so a spare plain Pico can act as an SWD probe when needed.

| Function | GPIO | Pin |
|---|---|---|
| NeoPixel ring data | GP2 | 4 |
| I2C SDA / SCL (SHT3x + LCD) | GP4 / GP5 | 6 / 7 |
| NeoPixel digits data | GP7 | 10 |
| Buzzer | GP9 | 12 |
| Rotary encoder SW / CLK / DT | GP13 / GP14 / GP15 | 17 / 19 / 20 |
| Arcade button / lamp | GP16 / GP17 | 21 / 22 |
| MiCS-5524 `A0` / `EN` | GP26 / GP27 | 31 / 32 |

Pins are spread deliberately: the final build is hand-soldered onto an Adafruit Mint Tin-Sized
PermaProto, so unrelated signals get a free pin between them. Pins from the same module stay
adjacent, since they arrive as one bundle.

Peripherals: SHT3x/4x temp+humidity, DFRobot Gravity LCD1602 RGB (DFR0464), 24-pixel NeoPixel ring,
32-pixel NeoPixel seven-segment (4 digits x 8 pixels, own data line), MiCS-5524 gas sensor, rotary encoder, arcade button, buzzer.

Powered from a 5 V Raspberry Pi supply via an Adafruit 4090 USB-C breakout. Pixel brightness is
clamped in software against a current budget — 56 pixels at full white would pull ~3.4 A.

## Building

```bash
./go.sh                # build
./go.sh flash          # build and load onto the board
./go.sh flash monitor  # ...and open the serial console
```

First flash needs `BOOTSEL` held while plugging in USB. After that the firmware exposes a reset
interface and `picotool` reboots the board itself.

## Configuration

```bash
cp src/config.h.example src/config.h
```

`src/config.h` holds WiFi credentials and the home coordinates the ISS compass points relative to.
It is gitignored and must stay that way.
