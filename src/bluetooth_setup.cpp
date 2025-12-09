/*
 * Mochi Robot - Bluetooth Setup Implementation
 */

#include "bluetooth_setup.h"
#include <Arduino.h>

BluetoothSetup::BluetoothSetup(Preferences* prefs) {
  preferences = prefs;
  isEnabled = false;
  isConnected = false;
  lastActivity = 0;
  commandBuffer = "";
}

bool BluetoothSetup::begin() {
  // ESP32-C3 doesn't support classic BluetoothSerial
  // Bluetooth setup disabled for now - use WiFi AP mode instead
  // TODO: Implement BLE setup
  Serial.println("⚠️ Bluetooth disabled - ESP32-C3 requires BLE (not yet implemented)");
  Serial.println("📡 Using WiFi AP mode for setup instead");
  isEnabled = false;
  return false;
}

void BluetoothSetup::stop() {
  // SerialBT = nullptr; // Disabled
  isEnabled = false;
  isConnected = false;
  Serial.println("📴 Bluetooth stopped");
}

void BluetoothSetup::update() {
  if (!isEnabled) {
    return;
  }
  
  // Bluetooth disabled - no updates
  // TODO: Implement BLE updates
  /*
  // Check for incoming data
  if (SerialBT->available()) {
    isConnected = true;
    lastActivity = millis();
    
    char c = SerialBT->read();
    
    if (c == '\n' || c == '\r') {
      if (commandBuffer.length() > 0) {
        Serial.print("📥 BT Command: ");
        Serial.println(commandBuffer);
        
        SetupData data;
        if (parseCommand(commandBuffer, &data)) {
          if (data.isValid) {
            saveSetupData(&data);
            sendResponse("OK: Data saved");
          } else {
            sendResponse("ERROR: Invalid command");
          }
        }
        
        commandBuffer = "";
      }
    } else {
      commandBuffer += c;
      
      // Prevent buffer overflow
      if (commandBuffer.length() > 200) {
        commandBuffer = "";
        sendResponse("ERROR: Command too long");
      }
    }
  }
  
  // Check timeout
  if (isConnected && (millis() - lastActivity) > TIMEOUT) {
    Serial.println("⏱️ Bluetooth setup timeout");
    isConnected = false;
      sendResponse("Timeout: Setup session ended");
  }
  */
}

bool BluetoothSetup::parseCommand(String command, SetupData* data) {
  data->isValid = false;
  
  command.trim();
  command.toUpperCase();
  
  // Parse SET_WIFI command
  if (command.startsWith("SET_WIFI:")) {
    String params = command.substring(9);
    int commaPos = params.indexOf(',');
    
    if (commaPos > 0) {
      data->wifiSSID = params.substring(0, commaPos);
      data->wifiPassword = params.substring(commaPos + 1);
      data->isValid = true;
      sendResponse("WiFi credentials received");
      return true;
    } else {
      sendResponse("ERROR: Invalid WiFi format. Use: SET_WIFI:ssid,password");
      return false;
    }
  }
  
  // Parse SET_LOCATION command
  if (command.startsWith("SET_LOCATION:")) {
    String params = command.substring(13);
    int commaPos = params.indexOf(',');
    
    if (commaPos > 0) {
      data->latitude = params.substring(0, commaPos).toFloat();
      data->longitude = params.substring(commaPos + 1).toFloat();
      data->isValid = true;
      sendResponse("Location received");
      return true;
    } else {
      sendResponse("ERROR: Invalid location format. Use: SET_LOCATION:lat,lon");
      return false;
    }
  }
  
  // Parse SET_API_KEY command
  if (command.startsWith("SET_API_KEY:")) {
    String apiKey = command.substring(12);
    if (apiKey.length() > 0) {
      data->weatherAPIKey = apiKey;
      data->isValid = true;
      sendResponse("API key received");
      return true;
    } else {
      sendResponse("ERROR: API key cannot be empty");
      return false;
    }
  }
  
  // Parse SAVE command
  if (command == "SAVE") {
    data->isValid = true;
    sendResponse("Saving all data...");
    return true;
  }
  
  // Unknown command
  sendResponse("ERROR: Unknown command");
  return false;
}

void BluetoothSetup::sendResponse(String message) {
  // Bluetooth disabled
  Serial.print("📤 BT Response (disabled): ");
  Serial.println(message);
}

bool BluetoothSetup::getSetupData(SetupData* data) {
  preferences->begin("mochi", true);
  
  data->wifiSSID = preferences->getString("ssid", "");
  data->wifiPassword = preferences->getString("pass", "");
  data->weatherAPIKey = preferences->getString("weather_key", "");
  data->latitude = preferences->getFloat("lat", 0.0);
  data->longitude = preferences->getFloat("lon", 0.0);
  
  preferences->end();
  
  data->isValid = (data->wifiSSID.length() > 0 || data->latitude != 0.0);
  return data->isValid;
}

void BluetoothSetup::saveSetupData(SetupData* data) {
  preferences->begin("mochi", false);
  
  if (data->wifiSSID.length() > 0) {
    preferences->putString("ssid", data->wifiSSID);
    preferences->putString("pass", data->wifiPassword);
    Serial.print("💾 Saved WiFi: ");
    Serial.println(data->wifiSSID);
  }
  
  if (data->weatherAPIKey.length() > 0) {
    preferences->putString("weather_key", data->weatherAPIKey);
    Serial.println("💾 Saved Weather API key");
  }
  
  if (data->latitude != 0.0 && data->longitude != 0.0) {
    preferences->putFloat("lat", data->latitude);
    preferences->putFloat("lon", data->longitude);
    Serial.print("💾 Saved Location: ");
    Serial.print(data->latitude, 6);
    Serial.print(", ");
    Serial.println(data->longitude, 6);
  }
  
  preferences->end();
  
  sendResponse("Data saved successfully");
}

