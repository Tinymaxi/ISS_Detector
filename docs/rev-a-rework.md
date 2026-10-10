# Rev A rework: the two sensors that came back wrong

Bring-up of the first fabricated board, 7–10 October 2026. Everything else works —
CO₂, gas, clock with its coin cell, LCD, buzzer, arcade button, the Pico on its sockets.
Two parts need rework, and **both faults are in footprints we drew ourselves**, not in
anything JLCPCB did. All five boards are identical, so each needs the same treatment.

## U7 — the light sensor

**Two separate mistakes.**

1. **Wrong variant.** The BOM says `VEML7700-TR` (LCSC `C504893`), which is the
   **side-view** part: it senses through its upright face. The design needs the window
   facing away from the board, which is the **top-view `VEML7700-TT`**. Vishay
   distinguish them only in a remark in the ordering table.
2. **Fitted 180° out.** The body sits above its pads; LCSC's footprint has the body
   above the pads at 0°, pin 1 right, and our board places it at 180°.

**What it did to the board.** Reversed, the chip's GND pin lands on +3V3 and its VDD on
GND. Its protection diodes then fed the 3V3 rail straight into SDA and SCL, clamping
both at about 2.4 V — one diode drop under the rail. Nothing could pull the bus low, so
**no I²C device could answer at all**: no CO₂ sensor, no clock, no LCD. Lifting U7 fixed
every one of them at once.

**The fix.**

- Remove the `-TR` part. Don't re-use it: it has been reverse-powered and got hot.
- Fit **`VEML7700-TT`** instead — DigiKey `751-VEML7700-TTCT-ND`, about CHF 1.27.
  Both variants use the same four pads at 1.27 mm pitch; Vishay's land patterns differ
  only in pad length, 1.4 against 1.6 mm, and ours are 1.8 mm long.
- **Pin 1 (SCL) goes on the left-hand pad**, the one beside the silkscreen dot. Then
  pin 2 VDD, pin 3 GND, pin 4 SDA.
- Check the body clears `C13` before soldering: lying flat it is 3 mm deep and there is
  about 3.5 mm of room.

**How to confirm:** the I²C scan should find **0x10** alongside 0x62 and 0x68.

## U6 — the PIR

**The footprint is mirrored.** Verified against LCSC's own footprint for `C389589`:
their pad ring and ours are mirror images, pin for pin. Our hole pattern is the real
lead pattern **rotated by −45°**, which is exactly the tilt the fitted sensor shows.

Because a mirror is not a rotation, **the part cannot be inserted correctly at all**.
It went in the only way it fits, and the leads land like this:

| Sensor pin | Lands on our pad | Which carries |
|---|---|---|
| 1 SENS | 3 | GND |
| 2 OEN | 2 | `PIR_EN`, GP22 — correct by luck |
| 3 VSS | 1 | the SENS divider, about 0.4 V |
| 4 VDD | 6 | `ONTIME`, tied to GND through R17 |
| 5 REL | 5 | `PIR_OUT`, GP21 — correct by luck |
| 6 ONTIME | 4 | +3V3_PIR |

So **the sensor's supply pin is grounded and its ground pin sits at 0.4 V**. It has never
been powered. The symptom is GP21 and GP22 both reading 0 and refusing to be driven
high, while both nets measure 20 MΩ to GND unpowered — the copper is clean, the part is
inert and clamping through its protection diodes.

**The fix, option 1: fit it from the other side.** Flipping a through-hole part mirrors
its lead pattern, and that cancels our mirror exactly — every pin then lands on the pad
with the right name, with no wires and nothing crossed. Push the leads up through from
the bottom and solder them on the top. The cost is mechanical: the dome then looks out
of the **bottom** face, so this only works if the case can give it a view that way.

There is no drop-in part to rescue us instead. `BS412` is a four-pin device in a square
pattern, and the six-pin Senba sensors all share the BS612 pinout, so none of them has
the mirrored ring we would need.

**The fix, option 2: wires.** Remove U6 and connect each sensor pin to the pad **of the
same name**, with six short wires or by crossing the leads in sleeving. The pads, in the
board's top view:

```
            6 ONTIME
   5 PIR_OUT        1 SENS
   4 +3V3_PIR       2 PIR_EN
            3 GND
```

Mounting it on a short flying lead may suit the build anyway: the sensor has to look out
through the front of the case behind its Fresnel lens, not lie flat on the board.

**How to confirm:** with the sensor off, GP21 can be driven high by the Pico. With it
wired correctly, the firmware prints `pir: motion` when you wave at it. The BS612 wants
**two pulses within four seconds** before it declares motion, so wave rather than swipe
once.

## For a rev B

- `eda/libraries/BS612.lbr`: **mirror the pad numbering**, and draw the can's flange tab
  on the silkscreen so the orientation is visible at a glance.
- `eda/libraries/VEML7700.lbr`: specify the **`-TT`** part, and draw the body outline to
  one side of the pad row rather than centred on it. The centred outline is what made
  JLCPCB's placement preview impossible to read.
- `eda/make-jlc-cpl.py`: add `LED-SMD_4P-L6.8-W2.4` to `ROTATION_FIX` once the correct
  value is confirmed against a preview.
- Check every footprint we drew ourselves against the manufacturer's **lead-side** view.
  Both faults here come from reading a bottom view as a top view.
