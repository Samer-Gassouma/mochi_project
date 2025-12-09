/*
 * Mochi Robot - BLE Setup Implementation (ESP32-C3)
 * Uses NimBLE-Arduino to expose a UART-like service for setup.
 */

#include "ble_setup.h"

// UUIDs for Nordic UART Service
static const char* SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* RX_UUID      = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* TX_UUID      = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

BleSetup::BleSetup(Preferences* prefs) {
  preferences = prefs;
  isEnabled = false;
  isConnected = false;
  hasNewData = false;
  server = nullptr;
  txChar = nullptr;
  rxChar = nullptr;
  cachedData = {};
}

bool BleSetup::begin() {
  NimBLEDevice::init("Mochi-Robot-Setup");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9); // max TX power

  server = NimBLEDevice::createServer();
  server->setCallbacks(new NimBLEServerCallbacks());

  NimBLEService* service = server->createService(SERVICE_UUID);

  txChar = service->createCharacteristic(
      TX_UUID,
      NIMBLE_PROPERTY::NOTIFY
  );

  rxChar = service->createCharacteristic(
      RX_UUID,
      NIMBLE_PROPERTY::WRITE
  );
  rxChar->setCallbacks(new BleRxCallbacks(this));

  service->start();

  NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
  advertising->addServiceUUID(service->getUUID());
  advertising->setScanResponse(true);
  advertising->start();

  isEnabled = true;
  Serial.println("✅ BLE setup advertising as 'Mochi-Robot-Setup'");

  // Load cached data from NVS
  preferences->begin("mochi", true);
  cachedData.wifiSSID = preferences->getString("ssid", "");
  cachedData.wifiPassword = preferences->getString("pass", "");
  cachedData.weatherAPIKey = preferences->getString("weather_key", "");
  cachedData.latitude = preferences->getFloat("lat", 0.0);
  cachedData.longitude = preferences->getFloat("lon", 0.0);
  preferences->end();
  cachedData.isValid = (cachedData.wifiSSID.length() > 0 || cachedData.latitude != 0.0);

  return true;
}

void BleSetup::stop() {
  if (isEnabled) {
    NimBLEDevice::stopAdvertising();
    NimBLEDevice::deinit(true);
    isEnabled = false;
    isConnected = false;
  }
}

bool BleSetup::parseJson(const std::string& payload, SetupData& data) {
  StaticJsonDocument<512> doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    Serial.print("❌ BLE JSON parse error: ");
    Serial.println(err.c_str());
    return false;
  }

  data.isValid = false;

  if (doc.containsKey("ssid")) {
    data.wifiSSID = doc["ssid"].as<String>();
  }
  if (doc.containsKey("password")) {
    data.wifiPassword = doc["password"].as<String>();
  }
  if (doc.containsKey("apiKey")) {
    data.weatherAPIKey = doc["apiKey"].as<String>();
  }
  if (doc.containsKey("lat")) {
    data.latitude = doc["lat"].as<float>();
  }
  if (doc.containsKey("lon")) {
    data.longitude = doc["lon"].as<float>();
  }

  data.isValid = (data.wifiSSID.length() > 0 || data.weatherAPIKey.length() > 0 || data.latitude != 0.0);
  return data.isValid;
}

void BleSetup::saveSetupData(const SetupData& data) {
  preferences->begin("mochi", false);
  if (data.wifiSSID.length() > 0) {
    preferences->putString("ssid", data.wifiSSID);
    preferences->putString("pass", data.wifiPassword);
  }
  if (data.weatherAPIKey.length() > 0) {
    preferences->putString("weather_key", data.weatherAPIKey);
  }
  if (data.latitude != 0.0 && data.longitude != 0.0) {
    preferences->putFloat("lat", data.latitude);
    preferences->putFloat("lon", data.longitude);
  }
  preferences->end();
  Serial.println("💾 BLE: Setup data saved to NVS");
}

bool BleSetup::getSetupData(SetupData* outData) {
  if (hasNewData) {
    *outData = cachedData;
    hasNewData = false;
    return true;
  }
  // If we already have valid cached data, return it (for initial load)
  if (cachedData.isValid) {
    *outData = cachedData;
    return true;
  }
  return false;
}

void BleSetup::sendResponse(const String& msg) {
  if (txChar) {
    txChar->setValue(msg);
    txChar->notify();
  }
  Serial.print("📤 BLE Response: ");
  Serial.println(msg);
}

void BleRxCallbacks::onWrite(NimBLECharacteristic* pCharacteristic) {
  std::string value = pCharacteristic->getValue();
  if (value.empty() || parent == nullptr) return;

  Serial.print("📥 BLE RX: ");
  Serial.println(value.c_str());

  SetupData data;
  if (parent->parseJson(value, data)) {
    parent->cachedData = data;
    parent->hasNewData = true;
    parent->saveSetupData(data);
    parent->sendResponse("OK");
  } else {
    parent->sendResponse("ERROR");
  }
}

