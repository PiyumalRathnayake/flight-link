# Flight Link

An ESP32-based remote control (RC) transmitter for aircraft and drones, featuring a touchscreen UI, dual-joystick controls, and a dual-mode (nRF24L01 + WiFi) radio link.

## Features

- **Touchscreen UI** — multi-screen interface built with LVGL on a TFT display (320×240).
- **Dual radio link** — transmits over an nRF24L01 module, WiFi UDP, or both.
- **Four-axis control** — throttle, rudder, aileron, and elevator from two analog joysticks.
- **Stick calibration** — guided center/limit calibration with deadzone and axis reversal, stored in non-volatile memory.
- **Binding** — pairs the transmitter with a receiver over a dedicated bind pipe.
- **Encryption** — optional payload encryption for the control link.
- **Device activation** — generates a unique device key from the ESP32 MAC address.
- **Power saving** — screen timeout after inactivity.

## Hardware

- ESP32 DevKit (`esp32doit-devkit-v1`)
- TFT display (driven via `TFT_eSPI`)
- nRF24L01 radio module
- Two analog joysticks (4 axes)

### Radio pin mapping

| Signal | Pin |
|--------|-----|
| SCK    | 25  |
| MISO   | 26  |
| MOSI   | 27  |
| CSN    | 14  |

## Project Structure

| Path | Description |
|------|-------------|
| `src/main.cpp` | Main application — UI handling, calibration, activation, and the control loop. |
| `lib/RC_Input` | Joystick input reading, calibration, and axis mapping. |
| `lib/RC_Radio` | nRF24L01 / WiFi transmission, binding, and encryption. |
| `lib/RC_Shared` | Shared packet structures (control, bind, telemetry). |
| `lib/UI` | LVGL UI screens and assets. |

## Build & Upload

This is a [PlatformIO](https://platformio.org/) project.

```bash
# Build
pio run

# Upload to the ESP32
pio run --target upload

# Open the serial monitor
pio device monitor
```

### Dependencies

Managed automatically by PlatformIO (`platformio.ini`):

- `bodmer/TFT_eSPI`
- `lvgl/lvgl@8.3.11`
- `nrf24/RF24`

## Author

**Piyumal Rathnayake**
