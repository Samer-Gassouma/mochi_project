# Mochi Robot - Wiring Diagram

## ESP32-C3 Pinout Connections

### Components Used
- **ESP32-C3 DevKitM-1** (or compatible ESP32-C3 board)
- **SSD1306 OLED Display** (128x64, I2C)
- **TTP223 Touch Sensor**
- **MAX98357A I2S Audio Amplifier**
- **Speaker** (8Ω, 0.5W or similar)

---

## Pin Connections

### 1. SSD1306 OLED Display (I2C)

| OLED Display | ESP32-C3 | Notes |
|--------------|----------|-------|
| VCC          | 3.3V     | Power supply |
| GND          | GND      | Ground |
| SDA          | GPIO 8   | I2C Data line |
| SCL          | GPIO 9   | I2C Clock line |

**I2C Address:** 0x3C (default)

---

### 2. TTP223 Touch Sensor

| TTP223 Pin | ESP32-C3 | Notes |
|------------|----------|-------|
| VCC        | 3.3V     | Power supply |
| GND        | GND      | Ground |
| I/O        | GPIO 2   | Digital input (HIGH when touched) |

**Note:** TTP223 outputs HIGH (3.3V) when touched, LOW (0V) when not touched.

---

### 3. MAX98357A I2S Audio Amplifier

| MAX98357A Pin | ESP32-C3 | Notes |
|---------------|----------|-------|
| Vin           | 5V       | Power supply (can use 3.3V but 5V recommended) |
| GND           | GND      | Ground |
| BCLK          | GPIO 4   | Bit Clock (I2S) |
| LRC           | GPIO 5   | Left/Right Clock / Word Select (I2S) |
| DIN           | GPIO 6   | Data Input (I2S) |
| SD            | GPIO 7   | Shutdown pin (HIGH = enabled, LOW = disabled) |

**Note:** SD pin is set HIGH in code to enable the amplifier.

---

### 4. Speaker

| Speaker Terminal | MAX98357A | Notes |
|------------------|-----------|-------|
| + (Positive)     | Speaker + | Audio output positive |
| - (Negative)     | Speaker - | Audio output negative |


```
ESP32-C3 Pin Layout:
┌─────────────────────────────────┐
│                                     │
│  GPIO 2  ←── TTP223 I/O            │
│  GPIO 4  ←── MAX98357A BCLK        │
│  GPIO 5  ←── MAX98357A LRC          │
│  GPIO 6  ←── MAX98357A DIN          │
│  GPIO 7  ←── MAX98357A SD           │
│  GPIO 8  ←── SSD1306 SDA (I2C)      │
│  GPIO 9  ←── SSD1306 SCL (I2C)      │
│                                     │
│  3.3V    ←── SSD1306 VCC            │
│  3.3V    ←── TTP223 VCC             │
│  5V      ←── MAX98357A Vin          │
│  GND     ←── All GND connections    │
│                                     │
└─────────────────────────────────┘
```

---

## Visual Connection Diagram

```
                    ┌─────────────┐
                    │   ESP32-C3  │
                    │  DevKitM-1  │
                    └──────┬──────┘
                           │
        ┌──────────────────┼──────────────────┐
        │                  │                  │
        │                  │                  │
    ┌───▼───┐         ┌───▼───┐         ┌───▼───┐
    │SSD1306│         │TTP223 │         │MAX98357│
    │OLED   │         │Touch  │         │Audio   │
    │       │         │Sensor │         │Amp     │
    └───────┘         └───────┘         └───┬───┘
                                             │
                                        ┌───▼───┐
                                        │Speaker│
                                        └───────┘

Power Connections:
- All VCC → 3.3V (except MAX98357A Vin → 5V)
- All GND → Common GND
```

---
