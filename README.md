# BatteryDiagnostic-tool

A pocket-sized battery voltage tester built on an **ESP32**, an **INA226** power monitor, and a **320×240 TFT display**, wrapped in a cyberpunk/HUD-style interface. Switch between battery types over **Bluetooth** and get a live voltage readout, estimated charge percentage, and a segmented power bar.

Originally built to check a car's 12 V battery, it now also handles LiPo packs, 9 V batteries, and AA/AAA cells.

<!-- Add a photo or GIF of your build here -->
<!-- ![Corolla HUD demo](docs/demo.jpg) -->

---

## Features

- **Live voltage readout** from the INA226 (updates every 150 ms)
- **Estimated battery percentage** based on per-chemistry voltage ranges
- **Color-coded status**: red at ≤ 20 %, cyan in the middle, green at ≥ 80 %
- **Flicker-free rendering** using an off-screen `TFT_eSprite` buffer
- **5 battery modes**, switchable wirelessly over Bluetooth Serial
- **Fatal-error screen** if the INA226 isn't detected on boot

---

## Hardware

| Component | Notes |
|---|---|
| ESP32 board with a 320×240 TFT | Code uses `TFT_eSPI`, rotation 1, inverted colors. An ESP32 "Cheap Yellow Display" style board works well |
| INA226 breakout | I²C address `0x40` (default) |
| Battery under test | Connected to the INA226 bus-voltage input |

### Wiring (I²C)

| INA226 | ESP32 |
|---|---|
| VCC | 3.3 V |
| GND | GND |
| SDA | GPIO 27 |
| SCL | GPIO 22 |

The battery being measured connects to the INA226's **VBUS** and **GND** pins. The INA226 bus input is rated for up to **36 V**, so it's fine for a 12 V car battery, but always double-check your wiring and polarity before connecting anything.

> Make sure the battery's ground and the ESP32's ground are common, and never exceed the INA226's voltage rating.

---

## Software Requirements

Arduino IDE or PlatformIO
ESP32 board support package
Libraries:
TFT_eSPI
INA226 (Rob Tillaart)
Wire and BluetoothSerial (included with the ESP32 core))

### TFT_eSPI configuration

`TFT_eSPI` must be configured for your specific display and pin layout, either by editing `User_Setup.h` in the library folder or by selecting the correct setup file. Without this, the screen will stay blank or show garbage.

---

## Getting Started

1. **Install the libraries** listed above.
2. **Configure `TFT_eSPI`** for your display.
3. **Select your ESP32 board** and COM port.
4. **Upload the sketch.**
5. Power up. You'll see the boot screen, then the HUD in `SYS.CAR_12V` mode.

---

## Bluetooth Control

The device advertises as **`Corolla_HUD`**. Pair with it from your phone or PC and use any Bluetooth serial terminal app (e.g., *Serial Bluetooth Terminal* on Android) to send single characters:

| Command | Action |
|---|---|
| `0` | Car 12 V battery |
| `1` | LiPo 2S |
| `2` | 9 V battery |
| `3` | AA cell |
| `4` | AAA cell |
| `n` / `N` | Cycle to the next mode (wraps from 4 back to 0) |

---

## Battery Modes & Voltage Ranges

Percentage is calculated as a linear interpolation between the minimum and maximum voltage for each mode, clamped to 0–100 %.

| Mode | HUD Title | Min V (0 %) | Max V (100 %) |
|---|---|---|---|
| 0 | `SYS.CAR_12V` | 11.9 V | 12.6 V |
| 1 | `SYS.LIPO_2S` | 6.4 V | 8.4 V |
| 2 | `SYS.PWR_9V` | 6.0 V | 9.5 V |
| 3 | `SYS.CELL_AA` | 1.0 V | 1.6 V |
| 4 | `SYS.CELL_AAA` | 1.0 V | 1.6 V |

You can tune these values in the `switch (currentMode)` block in `loop()` to match your batteries.

---

## Display Colors

| Charge level | Color |
|---|---|
| ≤ 20 % | Red (`ALERT_RED`) |
| 21 – 79 % | Cyan (`CYBER_CYAN`) |
| ≥ 80 % | Green (`NEON_GREEN`) |

---

## Limitations

- The percentage is a **rough estimate** derived from open-circuit-style voltage. Real state of charge varies with load, temperature, battery age, and chemistry (e.g., alkaline vs. NiMH for AA/AAA).
- For an accurate car-battery reading, measure with the engine off and the battery rested.
- Only bus voltage is read; current and power measurements from the INA226 aren't used yet.

---

## Ideas for Future Improvements

- Show current (A) and power (W) using the INA226 shunt readings (Feature added)
- Add NiMH / Li-ion 1S / LiFePO4 modes
- Add a physical button for mode switching
- Non-linear discharge curves for more accurate percentages (Feature added)
- Voltage logging over Bluetooth

---



