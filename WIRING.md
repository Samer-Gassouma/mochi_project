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

### 3. Buzzer (Passive or Active Piezo)

| Buzzer Pin | ESP32-C3 | Notes |
|------------|----------|-------|
| + (Positive) | GPIO 4 | Driven with PWM (LEDC) |
| - (Negative) | GND    | Common ground |

**Notes:**
- Use a 3.3V piezo buzzer. For louder 5V buzzers, drive through an NPN transistor + diode and power from 5V.
- The MAX98357A I2S amplifier and speaker are no longer used; GPIOs 5, 6, 7 are now free.


```
ESP32-C3 Pin Layout:
┌─────────────────────────────────┐
│                                     │
│  GPIO 2  ←── TTP223 I/O            │
│  GPIO 4  ←── Buzzer +              │
│  GPIO 8  ←── SSD1306 SDA (I2C)      │
│  GPIO 9  ←── SSD1306 SCL (I2C)      │
│                                     │
│  3.3V    ←── SSD1306 VCC            │
│  3.3V    ←── TTP223 VCC             │
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
                    │SSD1306│         │TTP223 │         │ Buzzer │
                    │OLED   │         │Touch  │         │ (Piezo)│
                    │       │         │Sensor │         │        │
                    └───────┘         └───────┘         └───┬───┘
                                             │
                                        ┌───▼───┐
                                        │ GND  │
                                        └───────┘

Power Connections:
- All VCC → 3.3V
- All GND → Common GND
```

---
