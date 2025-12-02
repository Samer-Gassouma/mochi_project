# Mochi Robot Project

An interactive pet robot built with ESP32-C3, featuring emotions, touch interactions, hunger/energy systems, and a mobile app for monitoring and control.

## 🎯 Project Overview

Mochi is a smart pet robot that:
- Displays emotions and behaviors on an OLED screen
- Responds to touch interactions
- Has a hunger and energy system
- Connects to WiFi for remote monitoring
- Can be controlled via a React Native mobile app

## 📁 Project Structure

```
mochi_project/
├── src/                    # ESP32-C3 source code
│   ├── main.cpp           # Main robot logic
│   ├── mochi_face.h/cpp   # Display and emotion rendering
│   └── emoji_drawer.h/cpp # Emoji drawing utilities
├── mochi-app/             # React Native Expo mobile app
│   ├── App.js             # Main app component
│   ├── android/           # Android native code
│   └── ios/               # iOS native code
├── test/                  # Component test files
├── platformio.ini         # PlatformIO configuration
├── WIRING.md              # Complete wiring diagram
└── README.md              # This file
```

## 🛠️ Hardware Components

- **ESP32-C3 DevKitM-1** (or compatible ESP32-C3 board)
- **SSD1306 OLED Display** (128x64, I2C)
- **TTP223 Touch Sensor**
- **MAX98357A I2S Audio Amplifier**
- **Speaker** (8Ω, 0.5W recommended)

See [WIRING.md](WIRING.md) for complete pin connections and wiring diagram.

## 🔌 Pin Connections

| Component | ESP32-C3 Pin |
|-----------|--------------|
| OLED SDA  | GPIO 8       |
| OLED SCL  | GPIO 9       |
| Touch I/O | GPIO 2       |
| Audio BCLK| GPIO 4       |
| Audio LRC | GPIO 5       |
| Audio DIN | GPIO 6       |
| Audio SD  | GPIO 7       |

## 📱 Mobile App

The React Native Expo app allows you to:
- Monitor robot status (emotion, hunger, energy)
- View touch count and last update time
- Configure WiFi settings
- Discover robot on local network
- Connect to robot via WiFi (AP mode or home network)

### App Setup

```bash
cd mochi-app
npm install
npx expo prebuild
npx expo run:android  # or run:ios
```

## 🚀 ESP32 Setup

### Prerequisites

- [PlatformIO](https://platformio.org/) installed
- ESP32-C3 board connected via USB

### Quick Start

**For Mac users**, see the detailed [MAC_SETUP_GUIDE.md](MAC_SETUP_GUIDE.md) for step-by-step instructions.

**For Linux/Windows users:**

```bash
# Build the project
pio run

# Upload to ESP32
pio run -t upload

# Monitor serial output
pio device monitor
```

## 🌐 WiFi Configuration

The ESP32 can operate in two modes:

1. **Access Point Mode** (default):
   - SSID: `Mochi-Robot`
   - Password: `mochi123`
   - IP: `192.168.4.1`

2. **WiFi Station Mode** (configured via app):
   - Connects to your home WiFi
   - Auto-connects on reboot
   - Discoverable via mDNS (`mochi-robot.local`)

### Network Discovery

The robot broadcasts its presence via:
- **mDNS**: `mochi-robot.local`
- **UDP Broadcast**: Port 8888

The mobile app automatically discovers the robot when on the same network.

## 📊 Features

### Emotions & Behaviors
- 😊 Happy, 😢 Sad, 😠 Angry, 😮 Surprised
- 😍 Love, 😴 Sleepy, 🤔 Thinking, 😂 Laughing
- 😋 Eating, 🍽️ Hungry, 😌 Full, 🤮 Throw Up
- ☠️ Starving, 😴 Sleeping, 🤒 Sick

### Touch Reactions
- Progressive touch responses (happy → love → annoyed)
- Touch count tracking
- Audio feedback

### Systems
- **Hunger System**: Decreases over time, affects behavior
- **Energy System**: Decreases over time, affects sleep
- **Random Behaviors**: Pet-like unpredictable actions
- **WiFi Status**: Visual indicator on display

## 🔧 Development

### ESP32 Code Structure

- `main.cpp`: Main program loop, WiFi, web server, state management
- `mochi_face.cpp`: Display rendering, emotion drawing, status display
- `emoji_drawer.cpp`: Emoji/emotion drawing utilities

### Mobile App Structure

- `App.js`: Main React component, WiFi communication, status polling
- Uses HTTP REST API for communication
- Auto-discovery via mDNS and IP scanning

## 📡 API Endpoints

The ESP32 web server provides:

- `GET /` - HTML dashboard
- `GET /status` - JSON status (emotion, hunger, energy, touchCount)
- `GET /wifi/info` - WiFi configuration status
- `POST /wifi/config` - Configure WiFi credentials

## 🐛 Troubleshooting

### Display not working
- Check I2C connections (SDA/SCL on GPIO 8/9)
- Verify power supply (3.3V)

### Touch sensor not responding
- Verify GPIO 2 connection
- Check TTP223 power (3.3V)

### No audio output
- Check I2S pin connections (GPIO 4/5/6/7)
- Verify MAX98357A power (5V recommended)
- Check speaker connections

### WiFi connection issues
- Ensure phone and ESP32 are on same network
- Try AP mode first (connect to "Mochi-Robot" network)
- Check serial monitor for WiFi status

## 📝 License

This project is open source. Feel free to use and modify as needed.

## 👤 Author

Samer Gassouma

## 🔗 Repository

[GitHub Repository](https://github.com/Samer-Gassouma/mochi_project)

