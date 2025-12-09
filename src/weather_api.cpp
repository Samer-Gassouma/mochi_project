/*
 * Mochi Robot - Weather API Implementation
 */

#include "weather_api.h"
#include <Arduino.h>

WeatherAPI::WeatherAPI(Preferences* prefs) {
  preferences = prefs;
  apiKey = "";
  // Hardcoded: Monastir, Tunisia
  latitude = 35.7784;
  longitude = 10.8262;
  lastUpdateTime = 0;
}

void WeatherAPI::setLocation(float lat, float lon) {
  latitude = lat;
  longitude = lon;
}

bool WeatherAPI::needsUpdate() {
  unsigned long now = millis();
  if (lastUpdateTime == 0 || (now - lastUpdateTime) > UPDATE_INTERVAL) {
    return true;
  }
  return false;
}

bool WeatherAPI::fetchWeather(WeatherData* data) {
  if (apiKey.length() == 0) {
    Serial.println("⚠️ Weather API key not set");
    return false;
  }
  
  if (!WiFi.isConnected()) {
    Serial.println("⚠️ WiFi not connected, loading cached weather");
    return loadCachedWeather(data);
  }
  
  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/weather?lat=";
  url += String(latitude, 6);
  url += "&lon=";
  url += String(longitude, 6);
  url += "&units=metric&appid=";
  url += apiKey;
  
  Serial.print("🌤️ Fetching weather from: ");
  Serial.println(url);
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println("✅ Weather API response received");
    
    if (parseWeatherResponse(payload, data)) {
      data->cached = false;
      data->lastUpdate = millis();
      lastUpdateTime = millis();
      saveCachedWeather(data);
      http.end();
      return true;
    }
  } else {
    Serial.print("❌ Weather API error: ");
    Serial.println(httpCode);
  }
  
  http.end();
  
  // Try to load cached data on failure
  return loadCachedWeather(data);
}

bool WeatherAPI::parseWeatherResponse(String json, WeatherData* data) {
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, json);
  
  if (error) {
    Serial.print("❌ JSON parse error: ");
    Serial.println(error.c_str());
    return false;
  }
  
  // Extract temperature
  if (doc.containsKey("main") && doc["main"].containsKey("temp")) {
    data->temperature = doc["main"]["temp"];
  } else {
    return false;
  }
  
  // Extract condition
  if (doc.containsKey("weather") && doc["weather"].is<JsonArray>()) {
    JsonArray weatherArray = doc["weather"].as<JsonArray>();
    if (weatherArray.size() > 0) {
      data->condition = weatherArray[0]["main"].as<String>();
      data->icon = weatherArray[0]["icon"].as<String>();
    }
  }
  
  // Map icon to emoji/symbol
  if (data->icon.length() > 0) {
    if (data->icon.startsWith("01")) {
      data->icon = "☀"; // Clear sky
    } else if (data->icon.startsWith("02")) {
      data->icon = "⛅"; // Few clouds
    } else if (data->icon.startsWith("03") || data->icon.startsWith("04")) {
      data->icon = "☁"; // Clouds
    } else if (data->icon.startsWith("09") || data->icon.startsWith("10")) {
      data->icon = "🌧"; // Rain
    } else if (data->icon.startsWith("11")) {
      data->icon = "⛈"; // Thunderstorm
    } else if (data->icon.startsWith("13")) {
      data->icon = "❄"; // Snow
    } else if (data->icon.startsWith("50")) {
      data->icon = "🌫"; // Mist
    } else {
      data->icon = "🌤"; // Default
    }
  }
  
  Serial.print("🌡️ Temperature: ");
  Serial.print(data->temperature);
  Serial.print("°C, Condition: ");
  Serial.println(data->condition);
  
  return true;
}

bool WeatherAPI::loadCachedWeather(WeatherData* data) {
  preferences->begin("mochi", true);
  data->temperature = preferences->getFloat("weather_temp", 0.0);
  data->condition = preferences->getString("weather_cond", "");
  data->icon = preferences->getString("weather_icon", "");
  data->lastUpdate = preferences->getULong64("weather_time", 0);
  preferences->end();
  
  if (data->temperature != 0.0 || data->condition.length() > 0) {
    data->cached = true;
    Serial.println("📦 Loaded cached weather data");
    return true;
  }
  
  return false;
}

void WeatherAPI::saveCachedWeather(WeatherData* data) {
  preferences->begin("mochi", false);
  preferences->putFloat("weather_temp", data->temperature);
  preferences->putString("weather_cond", data->condition);
  preferences->putString("weather_icon", data->icon);
  preferences->putULong64("weather_time", data->lastUpdate);
  preferences->end();
  
  Serial.println("💾 Saved weather data to cache");
}

