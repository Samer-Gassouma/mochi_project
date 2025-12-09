/*
 * Mochi Robot - BLE Setup Handler (ESP32-C3)
 * Exposes a BLE UART-like service for sending WiFi/location/API credentials.
 * Service UUID: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E (Nordic UART)
 * RX (write):   6E400002-B5A3-F393-E0A9-E50E24DCCA9E
 * TX (notify):  6E400003-B5A3-F393-E0A9-E50E24DCCA9E
 *
 * Expected payload (UTF-8 JSON) over RX:
 * {
 *   "ssid": "YOUR_WIFI",
 *   "password": "YOUR_PASS",
 *   "apiKey": "OPENWEATHER_KEY",
 *   "lat": 35.7784,
 *   "lon": 10.8262
 * }
 */

#ifndef BLE_SETUP_H
#define BLE_SETUP_H

#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <NimBLEDevice.h>

// Setup data structure (shared with main)
struct SetupData {
  String wifiSSID;
  String wifiPassword;
  String weatherAPIKey;
  float latitude;
  float longitude;
  bool isValid;
};

class BleSetup;

class BleRxCallbacks : public NimBLECharacteristicCallbacks {
public:
  BleRxCallbacks(BleSetup* parent) : parent(parent) {}
  void onWrite(NimBLECharacteristic* pCharacteristic) override;
private:
  BleSetup* parent;
};

class BleSetup {
public:
  BleSetup(Preferences* prefs);
  bool begin();
  void stop();
  void update() {} // no-op; kept for interface compatibility

  bool getIsEnabled() const { return isEnabled; }
  bool getIsConnected() const { return isConnected; }

  // Returns true if new data received since last call
  bool getSetupData(SetupData* outData);

private:
  Preferences* preferences;
  bool isEnabled;
  bool isConnected;
  bool hasNewData;
  SetupData cachedData;

  NimBLEServer* server;
  NimBLECharacteristic* txChar;
  NimBLECharacteristic* rxChar;

  void saveSetupData(const SetupData& data);
  void sendResponse(const String& msg);
  bool parseJson(const std::string& payload, SetupData& data);

  friend class BleRxCallbacks;
};

#endif

