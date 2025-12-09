/*
 * Mochi Robot - Screen Management System
 * Handles multiple screens: Robot Eyes, Clock, Prayer Time, Weather, Settings
 */

#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>

// Screen types
enum ScreenType {
  SCREEN_ROBOT_EYES = 0,
  SCREEN_CLOCK,
  SCREEN_PRAYER_TIME,
  SCREEN_WEATHER,
  SCREEN_SETTINGS,
  SCREEN_COUNT
};

class ScreenManager {
private:
  Adafruit_SSD1306* display;
  ScreenType currentScreen;
  unsigned long lastScreenUpdate;
  unsigned long screenUpdateInterval;
  
  // Clock screen
  struct tm timeInfo;
  bool timeSynced;
  
  // Prayer time screen
  String nextPrayerName;
  String nextPrayerTime;
  int minutesUntilPrayer;
  
  // Weather screen
  float temperature;
  String weatherCondition;
  String weatherIcon;
  bool weatherCached;
  
  // Settings screen
  int settingsPage;
  String lastWeatherUpdate;
  String lastPrayerUpdate;
  String lastNTPUpdate;
  String wifiSSID;
  String wifiIP;
  int wifiRSSI;
  bool bluetoothEnabled;
  
public:
  ScreenManager(Adafruit_SSD1306* disp);
  
  // Screen navigation
  void nextScreen();
  void setScreen(ScreenType screen);
  ScreenType getCurrentScreen() { return currentScreen; }
  
  // Update and draw
  void update();
  void draw();
  
  // Screen-specific drawing
  void drawRobotEyes();
  void drawClock();
  void drawPrayerTime();
  void drawWeather();
  void drawSettings();
  
  // Data setters
  void setTime(struct tm* timeInfo);
  void setTimeSynced(bool synced) { timeSynced = synced; }
  void setNextPrayer(String name, String time, int minutes);
  void setWeather(float temp, String condition, String icon, bool cached = false);
  void setLastWeatherUpdate(String time) { lastWeatherUpdate = time; }
  void setLastPrayerUpdate(String time) { lastPrayerUpdate = time; }
  void setLastNTPUpdate(String time) { lastNTPUpdate = time; }
  void setWiFiInfo(String ssid, String ip, int rssi);
  void setBluetoothEnabled(bool enabled) { bluetoothEnabled = enabled; }
  
  // Settings navigation
  void nextSettingsPage();
  int getSettingsPage() { return settingsPage; }
};

#endif

