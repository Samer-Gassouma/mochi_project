/*
 * Mochi Robot - Weather API Client
 * Fetches weather data from OpenWeatherMap API
 */

#ifndef WEATHER_API_H
#define WEATHER_API_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>

struct WeatherData {
  float temperature;
  String condition;
  String icon;
  bool cached;
  unsigned long lastUpdate;
};

class WeatherAPI {
private:
  String apiKey;
  float latitude;
  float longitude;
  Preferences* preferences;
  unsigned long lastUpdateTime;
  static const unsigned long UPDATE_INTERVAL = 1800000; // 30 minutes
  
  bool parseWeatherResponse(String json, WeatherData* data);
  
public:
  WeatherAPI(Preferences* prefs);
  
  // Set API key (from Bluetooth setup)
  void setAPIKey(String key) { apiKey = key; }
  
  // Set location (hardcoded to Monastir, Tunisia for now)
  void setLocation(float lat, float lon);
  
  // Fetch weather data
  bool fetchWeather(WeatherData* data);
  
  // Load cached weather from NVS
  bool loadCachedWeather(WeatherData* data);
  
  // Save weather to NVS cache
  void saveCachedWeather(WeatherData* data);
  
  // Check if update is needed
  bool needsUpdate();
};

#endif

