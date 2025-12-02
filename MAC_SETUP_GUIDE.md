# Mac Setup Guide - Uploading to ESP32-C3

This guide will help you set up and upload the Mochi Robot code to your ESP32-C3 board on macOS.

## 📋 Prerequisites

1. **macOS** (10.15 Catalina or later recommended)
2. **ESP32-C3 DevKitM-1** (or compatible board)
3. **USB-C cable** (for connecting ESP32-C3 to Mac)

## 🔧 Step 1: Install PlatformIO

PlatformIO is the development environment we use to build and upload code to the ESP32.

### Option A: Install via Homebrew (Recommended)

1. **Install Homebrew** (if not already installed):
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

2. **Install PlatformIO Core**:
   ```bash
   brew install platformio
   ```

3. **Verify installation**:
   ```bash
   pio --version
   ```

### Option B: Install via VS Code Extension

1. **Install Visual Studio Code**:
   - Download from: https://code.visualstudio.com/
   - Or via Homebrew: `brew install --cask visual-studio-code`

2. **Install PlatformIO IDE Extension**:
   - Open VS Code
   - Go to Extensions (⌘+Shift+X)
   - Search for "PlatformIO IDE"
   - Click "Install"

## 🔌 Step 2: Connect ESP32-C3 to Mac

1. **Connect the ESP32-C3** to your Mac using a USB-C cable
2. **Check if the board is detected**:
   ```bash
   ls /dev/cu.*
   ```
   
   You should see something like:
   - `/dev/cu.usbmodem*` (for ESP32-C3)
   - Or `/dev/cu.SLAB_USBtoUART*` (for some boards)

3. **Note the port name** - you'll need it if auto-detection doesn't work

## 📁 Step 3: Clone/Download the Project

1. **Navigate to your desired directory**:
   ```bash
   cd ~/Documents  # or wherever you want the project
   ```

2. **Clone the repository** (if using Git):
   ```bash
   git clone git@github.com:Samer-Gassouma/mochi_project.git
   cd mochi_project
   ```

   Or download the ZIP from GitHub and extract it.

## 🚀 Step 4: Build and Upload

### Using Command Line (Terminal)

1. **Navigate to the project directory**:
   ```bash
   cd /path/to/mochi_project
   ```

2. **Build the project**:
   ```bash
   pio run
   ```
   
   This will:
   - Download necessary libraries
   - Compile the code
   - Show any errors

3. **Upload to ESP32**:
   ```bash
   pio run -t upload
   ```
   
   PlatformIO should automatically detect your ESP32-C3. If it doesn't, you may need to specify the port:
   ```bash
   pio run -t upload --upload-port /dev/cu.usbmodem*
   ```
   (Replace `*` with your actual port number)

4. **Monitor serial output** (optional, to see debug messages):
   ```bash
   pio device monitor
   ```
   
   Or in a separate terminal:
   ```bash
   pio device monitor --baud 115200
   ```

### Using VS Code with PlatformIO Extension

1. **Open the project**:
   - Open VS Code
   - File → Open Folder → Select the `mochi_project` folder

2. **Wait for PlatformIO to initialize**:
   - PlatformIO will automatically detect the project
   - It will download libraries and set up the environment (first time only)

3. **Build the project**:
   - Click the checkmark icon (✓) in the PlatformIO toolbar at the bottom
   - Or use the shortcut: ⌘+Option+B
   - Or go to: PlatformIO → Build

4. **Upload to ESP32**:
   - Click the arrow icon (→) in the PlatformIO toolbar
   - Or use the shortcut: ⌘+Option+U
   - Or go to: PlatformIO → Upload

5. **Monitor serial output**:
   - Click the plug icon (🔌) in the PlatformIO toolbar
   - Or go to: PlatformIO → Monitor

## 🔍 Step 5: Verify Upload

After uploading, you should see:

1. **On the OLED display**:
   - Mochi's face with emotion
   - Hunger and Energy percentages
   - WiFi icon (crossed out if in AP mode)

2. **In Serial Monitor** (if monitoring):
   ```
   Mochi Robot Starting...
   Display initialized
   Touch sensor initialized
   Audio initialized
   WiFi AP started: Mochi-Robot
   IP: 192.168.4.1
   ```

3. **WiFi Network**:
   - Look for a WiFi network named "Mochi-Robot"
   - Password: `mochi123`

## 🐛 Troubleshooting

### Problem: "No device found" or "Upload failed"

**Solutions:**
1. **Check USB cable**: Use a data-capable USB cable (not charge-only)
2. **Check USB port**: Try a different USB port on your Mac
3. **Check permissions**: macOS may need permission to access the USB device
   - System Preferences → Security & Privacy → Privacy → Full Disk Access
   - Add Terminal or VS Code if needed

4. **Install USB drivers** (if needed):
   - Some ESP32 boards need CH340 or CP2102 drivers
   - Download from:
     - CH340: https://github.com/WCHSoftGroup/ch34xser_macos
     - CP2102: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers

5. **Manually specify port**:
   ```bash
   pio run -t upload --upload-port /dev/cu.usbmodem14101
   ```
   (Replace with your actual port)

### Problem: "Permission denied" when accessing serial port

**Solution:**
Add your user to the `dialout` group (Linux) or use `sudo` (not recommended):
```bash
sudo pio run -t upload
```

On macOS, you may need to grant Terminal/VS Code Full Disk Access in System Preferences.

### Problem: PlatformIO not found

**Solutions:**
1. **Check PATH**: Make sure PlatformIO is in your PATH
   ```bash
   echo $PATH
   which pio
   ```

2. **Reinstall PlatformIO**:
   ```bash
   brew reinstall platformio
   ```

3. **Use full path**:
   ```bash
   ~/.platformio/penv/bin/pio run -t upload
   ```

### Problem: Libraries not downloading

**Solutions:**
1. **Check internet connection**
2. **Clear PlatformIO cache**:
   ```bash
   pio cache clean
   ```

3. **Manually install libraries**:
   ```bash
   pio lib install
   ```

### Problem: Build errors

**Common issues:**
1. **Wrong board selected**: Make sure `platformio.ini` has:
   ```ini
   board = esp32-c3-devkitm-1
   ```

2. **Missing libraries**: PlatformIO should auto-install, but you can manually install:
   ```bash
   pio lib install "Adafruit SSD1306"
   pio lib install "Adafruit GFX Library"
   ```

3. **Check `platformio.ini`**: Ensure all required libraries are listed in `lib_deps`

### Problem: ESP32 keeps resetting or not working

**Solutions:**
1. **Check power supply**: ESP32-C3 needs stable 5V via USB
2. **Check wiring**: Verify all connections (see WIRING.md)
3. **Check serial monitor**: Look for error messages
4. **Try uploading again**: Sometimes a second upload fixes issues

### Problem: Can't find the serial port

**Solutions:**
1. **List all serial ports**:
   ```bash
   ls /dev/cu.*
   ```

2. **Check System Information**:
   - Apple menu → About This Mac → System Report
   - Hardware → USB
   - Look for your ESP32 board

3. **Unplug and replug** the USB cable
4. **Restart your Mac** (if nothing else works)

## 📝 Quick Reference Commands

```bash
# Navigate to project
cd /path/to/mochi_project

# Build project
pio run

# Upload to ESP32
pio run -t upload

# Monitor serial output
pio device monitor

# Clean build files
pio run -t clean

# List connected devices
pio device list

# Check PlatformIO version
pio --version
```

## 🎯 Next Steps

After successfully uploading:

1. **Test the display**: You should see Mochi's face
2. **Test touch sensor**: Touch the sensor and watch the emotion change
3. **Test WiFi**: Connect to "Mochi-Robot" network
4. **Test mobile app**: Use the React Native app to connect and monitor

## 💡 Tips

- **Keep serial monitor open** while developing to see debug messages
- **Use VS Code** for easier development (syntax highlighting, auto-complete)
- **Save frequently** - PlatformIO will auto-build on save if configured
- **Check WIRING.md** if hardware isn't working
- **Use `pio run -t upload -v`** for verbose output if upload fails

## 📞 Need Help?

- Check the main [README.md](README.md) for project overview
- Check [WIRING.md](WIRING.md) for hardware connections
- Check serial monitor output for error messages
- Review PlatformIO documentation: https://docs.platformio.org/

---

**Happy coding! 🚀**

