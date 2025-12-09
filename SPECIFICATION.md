# Mochi Robot - Complete Specification

**Version:** 1.0  
**Last Updated:** 2025-01-27  
**Status:** Reference Document for All Development

---

## 💡 Hardware Setup

### Microcontroller
- **ESP32-C3 DevKitM-1**

### Components
- **Display:** SSD1306 OLED (128x64, I2C)
- **Touch Sensor:** TTP223 (digital output, HIGH when touched)
- **Audio:** MAX98357A I2S Amp (used as simple buzzer, no real speaker for now)
- **Connectivity:** Bluetooth + WiFi

### Library
- **@RoboEyes** for displaying robot eyes & emotions

---

## 📌 Pin Mapping

### OLED (I2C)
- **SDA** → GPIO 8
- **SCL** → GPIO 9
- **VCC** → 3.3V
- **GND** → GND
- **I2C Address:** 0x3C

### Touch Sensor (TTP223)
- **I/O** → GPIO 2
- **VCC** → 3.3V
- **GND** → GND

### MAX98357A (acting as buzzer)
- **BCLK** → GPIO 4
- **LRC** → GPIO 5
- **DIN** → GPIO 6
- **SD** → GPIO 7 (set HIGH to enable)
- **Vin** → 5V
- **GND** → GND

---

## 🎨 Main Screens

Mochi cycles between screens using the touch sensor:

### 1. Default Screen – Robot Eyes
- Uses **@RoboEyes** library
- Eyes reflect emotion + robot status
- Default screen when idle

### 2. Clock Screen
- Shows current time (NTP synchronized)
- Format: HH:MM or HH:MM:SS
- Updates every second

### 3. Prayer Time Screen
- Shows next Muslim prayer name
- Shows time remaining until next prayer
- Updates based on location and current time

### 4. Weather Screen
- Shows current temperature
- Shows weather condition (text)
- Shows weather icon (simplified ASCII/emoji representation)
- Updates from weather API

---

## 👆 Touch Behavior

### Single Tap
- **Action:** Next screen
- **Cycle:** Robot Eyes → Clock → Prayer Time → Weather → Robot Eyes (loop)

### Double Tap
- **Action:** Special emotion animation
- **Behavior:** Play excited/happy animation with sound

### Long Touch (1.5 seconds)
- **Action:** Open settings screen
- **Settings Screen Shows:**
  - WiFi connection status
  - Last API update times
  - Saved location (GPS coordinates)
  - Signal strength (RSSI)
  - Firmware version
  - Bluetooth pairing option
  - "Reset device" option

---

## 🚀 Bluetooth Setup Flow

**Used only for device setup** (not for ongoing communication)

### Flow:
1. User connects via Bluetooth from mobile app
2. App sends:
   - **WiFi SSID** + **password**
   - **User location** (GPS coordinates: latitude, longitude)
3. ESP32 caches these values in non-volatile memory (Preferences/NVS)
4. ESP32 connects to WiFi
5. ESP32 fetches:
   - Weather API data
   - Prayer times API data
   - Time (NTP sync)
6. All data stored in NVS for offline use

### Bluetooth Library
- Use ESP32 BluetoothSerial or BLE (preferred for mobile apps)

---

## 🌐 WiFi + API Logic

### API Refresh Intervals
- **Weather:** Every 30 minutes
- **Prayer Times:** Every 1 hour
- **Time Sync (NTP):** Every 12 hours

### Offline Behavior
- If no internet: use cached data from NVS
- Offline = switch robot eyes to "worried/sad" emotion
- Display cached data with indicator (e.g., "Cached" text)

### APIs Required
- **Weather API:** OpenWeatherMap or similar (requires API key)
- **Prayer Times API:** Aladhan API or similar (free, no key needed)
- **NTP Server:** pool.ntp.org or time.google.com

---

## 🤖 Emotion System

### Emotion Changes Based On:
1. **Time of day** (morning = happy, night = sleepy)
2. **Weather** (sunny = happy, rainy = sad)
3. **User interaction** (taps = excited)
4. **Connectivity status** (offline = worried/sad)
5. **Inactivity** (no touch for X minutes = bored/idle)

### Available Emotions (RoboEyes Library)
- **Happy** (HAPPY mood)
- **Sleepy** (TIRED mood)
- **Sad** (custom via position/animations)
- **Angry** (ANGRY mood)
- **Excited** (HAPPY + animations)
- **Idle/Bored** (DEFAULT + idle mode)
- **Neutral** (DEFAULT)

### Idle Animations
- **Blinking** (auto-blinker enabled)
- **Looking around** (idle mode - random eye positions)
- **Random cute behavior** (confused, laugh animations)

### Emotion Mapping
- Use RoboEyes `setMood()` for: DEFAULT, TIRED, ANGRY, HAPPY
- Use RoboEyes `setIdleMode()` for idle behavior
- Use RoboEyes `setAutoblinker()` for natural blinking
- Use RoboEyes `anim_confused()` and `anim_laugh()` for special animations

---

## 🔊 Audio Behavior (MAX98357A as Buzzer)

**For now, MAX98357A only plays simple tones** (no real sound effects).

### I2S-Generated Tones For:
- **Tap click sound** (short beep, ~200ms, 400Hz)
- **Wake-up beep** (rising tone, ~300ms)
- **Sleep beep** (falling tone, ~300ms)
- **Error beep** (low tone, ~400ms, 200Hz)
- **Prayer reminder beep** (gentle chime, ~500ms)
- **Emotion tones:**
  - Happy: short high tone (~150ms, 600Hz)
  - Sad: low tone (~200ms, 300Hz)
  - Excited: quick double beep

**Later:** When real speaker is added, replace tones with audio samples.

---

## 💤 Sleep/Wake System

### Sleep Trigger
- After **X minutes** with no interaction (configurable, default: 5 minutes)
- **Sleep Sequence:**
  1. Dim OLED (reduce contrast gradually)
  2. Eyes slowly close (RoboEyes `close()`)
  3. Play soft sleep beep
  4. Enter light sleep mode (ESP32 light sleep)

### Wake Trigger
- **On touch:**
  1. Wake up animation (eyes open - RoboEyes `open()`)
  2. Brighten OLED (restore contrast)
  3. Wake beep
  4. Resume normal operation

### Sleep State
- Use ESP32 light sleep to save power
- Keep WiFi/Bluetooth in low power mode
- Wake on GPIO interrupt (touch sensor)

---

## ⚙️ Settings Mode (Long Press)

### Settings Screen Shows:
1. **WiFi Status:**
   - Connected/Disconnected
   - SSID name
   - IP address
   - Signal strength (RSSI in dBm)

2. **Last API Update Times:**
   - Weather: "Last updated: HH:MM"
   - Prayer Times: "Last updated: HH:MM"
   - NTP: "Last synced: HH:MM"

3. **Location:**
   - Latitude: XX.XXXX
   - Longitude: YY.YYYY

4. **System Info:**
   - Firmware version
   - Uptime
   - Free heap memory

5. **Options:**
   - "Bluetooth Pairing" (re-enable BT for setup)
   - "Reset Device" (clear all saved data, restart)

### Navigation
- **Touch:** Cycle through settings items
- **Long Press:** Exit settings (return to default screen)

---

## 📦 Required Software Architecture

### State Machine
Clean state machine managing:
- **Screens:** Robot Eyes, Clock, Prayer Time, Weather, Settings
- **Emotions:** Happy, Sleepy, Sad, Angry, Excited, Idle, Neutral
- **Sleep/Wake:** Awake, Sleeping, Waking
- **Bluetooth Mode:** Setup mode, Normal mode
- **API Update Flow:** Idle, Fetching, Error, Success

### Code Structure
- **Modular code:** Separate files for:
  - Screen management
  - Emotion system
  - API handlers (Weather, Prayer, NTP)
  - Bluetooth setup
  - Touch handling
  - Audio (tone generation)
  - Sleep/wake logic

### Non-Blocking Code
- **Use `millis()` instead of `delay()`**
- All operations must be non-blocking
- Use state machines for multi-step operations

### Data Storage
- **NVS (Preferences):**
  - WiFi credentials (SSID, password)
  - Location (lat, lon)
  - Cached weather data
  - Cached prayer times
  - Last update timestamps
  - Settings (sleep timeout, etc.)

### OTA Firmware Update
- Support Over-The-Air updates via WiFi
- Use ESP32 OTA library
- Version checking and rollback capability

### I2S Tone Generator
- Clean I2S tone generation function
- Non-blocking (use state machine for long tones)
- Support for frequency, duration, volume

### RoboEyes Integration
- Replace current `MochiFace` text-based system
- Use RoboEyes library for all eye animations
- Map emotions to RoboEyes moods and animations

---

## 🔄 Current Implementation vs Specification

### What Needs to Change:
1. **Replace MochiFace with RoboEyes** - Currently using text-based emotion display
2. **Add Multi-Screen System** - Currently only shows emotion text
3. **Replace WiFi Web Server with Bluetooth Setup** - Currently uses web server for WiFi config
4. **Add NTP Time Sync** - No time synchronization currently
5. **Add Weather API** - No weather integration
6. **Add Prayer Times API** - No prayer times integration
7. **Refactor Touch Handling** - Currently uses feeding system, needs screen switching
8. **Add Sleep/Wake System** - No sleep mode currently
9. **Add Settings Screen** - No settings screen
10. **Update Emotion System** - Map to RoboEyes moods instead of text

### What Can Be Reused:
- Hardware pin mappings (already correct)
- I2S audio setup (already working)
- Touch sensor reading (needs refactoring for tap detection)
- WiFi connection logic (needs to be triggered by Bluetooth instead)
- NVS storage system (already in use)

---

## 📝 Development Notes

### Priority Order:
1. **Phase 1:** Integrate RoboEyes library, replace MochiFace
2. **Phase 2:** Implement multi-screen system with touch navigation
3. **Phase 3:** Add Bluetooth setup flow (replace WiFi web server)
4. **Phase 4:** Add NTP time sync and clock screen
5. **Phase 5:** Add Weather API and weather screen
6. **Phase 6:** Add Prayer Times API and prayer screen
7. **Phase 7:** Implement sleep/wake system
8. **Phase 8:** Add settings screen
9. **Phase 9:** Refine emotion system based on all factors
10. **Phase 10:** OTA update support

### Testing Checklist:
- [ ] RoboEyes displays correctly on OLED
- [ ] Touch single tap cycles screens
- [ ] Touch double tap triggers animation
- [ ] Long press opens settings
- [ ] Bluetooth setup flow works end-to-end
- [ ] WiFi connects after Bluetooth setup
- [ ] NTP syncs time correctly
- [ ] Weather API fetches and displays
- [ ] Prayer times API fetches and displays
- [ ] Offline mode uses cached data
- [ ] Sleep/wake cycle works
- [ ] Settings screen displays all info
- [ ] Audio tones play correctly
- [ ] Emotions change based on conditions

---

## 📚 API References

### RoboEyes Library
- Template class: `RoboEyes<Adafruit_SSD1306>`
- Main methods: `begin()`, `update()`, `setMood()`, `setPosition()`, `setAutoblinker()`, `setIdleMode()`, `anim_confused()`, `anim_laugh()`, `blink()`, `open()`, `close()`

### ESP32-C3
- Bluetooth: `BluetoothSerial` or `BLEDevice` (BLE preferred)
- WiFi: `WiFi.h`
- NTP: `time.h` with `configTime()`
- HTTP: `HTTPClient.h` or `WiFiClientSecure.h`
- NVS: `Preferences.h`
- Sleep: `esp_sleep.h`

### APIs
- **Weather:** OpenWeatherMap API (https://openweathermap.org/api)
- **Prayer Times:** Aladhan API (https://aladhan.com/prayer-times-api)
- **NTP:** pool.ntp.org or time.google.com

---

**End of Specification**

