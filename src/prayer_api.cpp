/*
 * Mochi Robot - Prayer Times API Implementation
 */

#include "prayer_api.h"
#include <Arduino.h>
#include <time.h>

PrayerAPI::PrayerAPI(Preferences* prefs) {
  preferences = prefs;
  // Hardcoded: Monastir, Tunisia
  latitude = 35.7784;
  longitude = 10.8262;
  lastUpdateTime = 0;
}

void PrayerAPI::setLocation(float lat, float lon) {
  latitude = lat;
  longitude = lon;
}

bool PrayerAPI::needsUpdate() {
  unsigned long now = millis();
  if (lastUpdateTime == 0 || (now - lastUpdateTime) > UPDATE_INTERVAL) {
    return true;
  }
  return false;
}

bool PrayerAPI::fetchPrayerTimes(PrayerData* data) {
  if (!WiFi.isConnected()) {
    Serial.println("⚠️ WiFi not connected, loading cached prayer times");
    return loadCachedPrayerTimes(data);
  }
  
  HTTPClient http;
  String url = "http://api.aladhan.com/v1/timings/";
  
  // Get today's date
  struct tm timeInfo;
  if (!getLocalTime(&timeInfo)) {
    Serial.println("❌ Cannot get local time for prayer API");
    return loadCachedPrayerTimes(data);
  }
  
  char dateStr[11];
  strftime(dateStr, sizeof(dateStr), "%d-%m-%Y", &timeInfo);
  
  url += String(dateStr);
  url += "?latitude=";
  url += String(latitude, 6);
  url += "&longitude=";
  url += String(longitude, 6);
  url += "&method=2"; // Islamic Society of North America method
  
  Serial.print("🕌 Fetching prayer times from: ");
  Serial.println(url);
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println("✅ Prayer API response received");
    
    if (parsePrayerResponse(payload, data)) {
      data->lastUpdate = millis();
      lastUpdateTime = millis();
      saveCachedPrayerTimes(data);
      updateNextPrayer(data);
      http.end();
      return true;
    }
  } else {
    Serial.print("❌ Prayer API error: ");
    Serial.println(httpCode);
  }
  
  http.end();
  
  // Try to load cached data on failure
  if (loadCachedPrayerTimes(data)) {
    updateNextPrayer(data);
    return true;
  }
  
  return false;
}

bool PrayerAPI::parsePrayerResponse(String json, PrayerData* data) {
  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, json);
  
  if (error) {
    Serial.print("❌ JSON parse error: ");
    Serial.println(error.c_str());
    return false;
  }
  
  // Extract prayer times
  if (!doc.containsKey("data") || !doc["data"].containsKey("timings")) {
    Serial.println("❌ Invalid prayer API response structure");
    return false;
  }
  
  JsonObject timings = doc["data"]["timings"];
  
  // Prayer names in order
  const char* prayerNames[] = {"Fajr", "Dhuhr", "Asr", "Maghrib", "Isha"};
  const char* prayerKeys[] = {"Fajr", "Dhuhr", "Asr", "Maghrib", "Isha"};
  
  for (int i = 0; i < 5; i++) {
    if (timings.containsKey(prayerKeys[i])) {
      String timeStr = timings[prayerKeys[i]].as<String>();
      data->prayers[i].name = prayerNames[i];
      data->prayers[i].time = timeStr;
      
      // Parse hour and minute
      int colonPos = timeStr.indexOf(':');
      if (colonPos > 0) {
        data->prayers[i].hour = timeStr.substring(0, colonPos).toInt();
        data->prayers[i].minute = timeStr.substring(colonPos + 1).toInt();
      }
    }
  }
  
  Serial.println("✅ Prayer times parsed successfully");
  return true;
}

void PrayerAPI::calculateNextPrayer(PrayerData* data, struct tm* currentTime) {
  int currentHour = currentTime->tm_hour;
  int currentMinute = currentTime->tm_min;
  int currentMinutes = currentHour * 60 + currentMinute;
  
  int nextPrayerIndex = -1;
  int minMinutesUntil = 1440; // 24 hours in minutes
  
  for (int i = 0; i < 5; i++) {
    int prayerMinutes = data->prayers[i].hour * 60 + data->prayers[i].minute;
    int minutesUntil = prayerMinutes - currentMinutes;
    
    // If prayer already passed today, check tomorrow
    if (minutesUntil < 0) {
      minutesUntil += 1440;
    }
    
    if (minutesUntil < minMinutesUntil) {
      minMinutesUntil = minutesUntil;
      nextPrayerIndex = i;
    }
  }
  
  if (nextPrayerIndex >= 0) {
    data->nextPrayerName = data->prayers[nextPrayerIndex].name;
    data->nextPrayerTime = data->prayers[nextPrayerIndex].time;
    data->minutesUntilNext = minMinutesUntil;
    
    Serial.print("🕌 Next prayer: ");
    Serial.print(data->nextPrayerName);
    Serial.print(" at ");
    Serial.print(data->nextPrayerTime);
    Serial.print(" (in ");
    Serial.print(data->minutesUntilNext);
    Serial.println(" minutes)");
  }
}

void PrayerAPI::updateNextPrayer(PrayerData* data) {
  struct tm timeInfo;
  if (getLocalTime(&timeInfo)) {
    calculateNextPrayer(data, &timeInfo);
  }
}

bool PrayerAPI::loadCachedPrayerTimes(PrayerData* data) {
  preferences->begin("mochi", true);
  
  const char* prayerNames[] = {"Fajr", "Dhuhr", "Asr", "Maghrib", "Isha"};
  
  for (int i = 0; i < 5; i++) {
    String key = "prayer_" + String(i) + "_time";
    data->prayers[i].time = preferences->getString(key.c_str(), "");
    data->prayers[i].name = prayerNames[i];
    
    // Parse hour and minute
    int colonPos = data->prayers[i].time.indexOf(':');
    if (colonPos > 0) {
      data->prayers[i].hour = data->prayers[i].time.substring(0, colonPos).toInt();
      data->prayers[i].minute = data->prayers[i].time.substring(colonPos + 1).toInt();
    }
  }
  
  data->lastUpdate = preferences->getULong64("prayer_time", 0);
  preferences->end();
  
  if (data->prayers[0].time.length() > 0) {
    Serial.println("📦 Loaded cached prayer times");
    return true;
  }
  
  return false;
}

void PrayerAPI::saveCachedPrayerTimes(PrayerData* data) {
  preferences->begin("mochi", false);
  
  for (int i = 0; i < 5; i++) {
    String key = "prayer_" + String(i) + "_time";
    preferences->putString(key.c_str(), data->prayers[i].time);
  }
  
  preferences->putULong64("prayer_time", data->lastUpdate);
  preferences->end();
  
  Serial.println("💾 Saved prayer times to cache");
}

