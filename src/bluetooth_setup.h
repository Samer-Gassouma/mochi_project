/*
 * Mochi Robot - Bluetooth Setup Handler
 * Handles WiFi credentials and location setup via Bluetooth
 */

#ifndef BLUETOOTH_SETUP_H
#define BLUETOOTH_SETUP_H

// ESP32-C3 only supports BLE, not classic BluetoothSerial
// For now, disable Bluetooth and use WiFi AP mode for setup
// TODO: Implement BLE setup later
// #include <BluetoothSerial.h>
#include <Preferences.h>

// Setup data structure
struct SetupData {
  String wifiSSID;
  String wifiPassword;
  String weatherAPIKey;
  float latitude;
  float longitude;
  bool isValid;
};

class BluetoothSetup {
private:
  // BluetoothSerial* SerialBT; // Disabled - ESP32-C3 doesn't support classic BT
  Preferences* preferences;
  bool isEnabled;
  bool isConnected;
  unsigned long lastActivity;
  static const unsigned long TIMEOUT = 300000; // 5 minutes timeout
  
  // Command parsing
  String commandBuffer;
  bool parseCommand(String command, SetupData* data);
  void sendResponse(String message);
  
public:
  BluetoothSetup(Preferences* prefs);
  
  // Initialize Bluetooth
  bool begin();
  void stop();
  
  // Update (call in loop)
  void update();
  
  // Check if setup is active
  bool getIsEnabled() { return isEnabled; }
  bool getIsConnected() { return isConnected; }
  
  // Get setup data
  bool getSetupData(SetupData* data);
  
  // Save setup data to NVS
  void saveSetupData(SetupData* data);
};

#endif

