/*
 * Mochi Robot - Prayer Times API Client
 * Fetches prayer times from Aladhan API
 */

#ifndef PRAYER_API_H
#define PRAYER_API_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <time.h>

struct PrayerTime {
  String name;
  String time;
  int hour;
  int minute;
};

struct PrayerData {
  PrayerTime prayers[5]; // Fajr, Dhuhr, Asr, Maghrib, Isha
  String nextPrayerName;
  String nextPrayerTime;
  int minutesUntilNext;
  unsigned long lastUpdate;
};

class PrayerAPI {
private:
  float latitude;
  float longitude;
  Preferences* preferences;
  unsigned long lastUpdateTime;
  static const unsigned long UPDATE_INTERVAL = 3600000; // 1 hour
  
  bool parsePrayerResponse(String json, PrayerData* data);
  void calculateNextPrayer(PrayerData* data, struct tm* currentTime);
  
public:
  PrayerAPI(Preferences* prefs);
  
  // Set location (hardcoded to Monastir, Tunisia for now)
  void setLocation(float lat, float lon);
  
  // Fetch prayer times
  bool fetchPrayerTimes(PrayerData* data);
  
  // Load cached prayer times from NVS
  bool loadCachedPrayerTimes(PrayerData* data);
  
  // Save prayer times to NVS cache
  void saveCachedPrayerTimes(PrayerData* data);
  
  // Update next prayer calculation
  void updateNextPrayer(PrayerData* data);
  
  // Check if update is needed
  bool needsUpdate();
};

#endif

