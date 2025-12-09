/*
 * Mochi Robot - Screen Management Implementation
 */

#include "screen_manager.h"
#include <Arduino.h>
#include <time.h>

ScreenManager::ScreenManager(Adafruit_SSD1306* disp) {
  display = disp;
  currentScreen = SCREEN_ROBOT_EYES;
  lastScreenUpdate = 0;
  screenUpdateInterval = 100; // Update every 100ms
  timeSynced = false;
  settingsPage = 0;
  temperature = 0.0;
  weatherCached = false;
  minutesUntilPrayer = 0;
  wifiRSSI = 0;
  bluetoothEnabled = false;
}

void ScreenManager::nextScreen() {
  // Cycle through screens: Robot Eyes -> Clock -> Prayer -> Weather -> Settings -> Robot Eyes
  currentScreen = (ScreenType)((currentScreen + 1) % SCREEN_COUNT);
  lastScreenUpdate = 0; // Force immediate update
}

void ScreenManager::setScreen(ScreenType screen) {
  if (screen < SCREEN_COUNT) {
    currentScreen = screen;
    lastScreenUpdate = 0;
  }
}

void ScreenManager::update() {
  unsigned long now = millis();
  if (now - lastScreenUpdate >= screenUpdateInterval) {
    lastScreenUpdate = now;
    draw();
  }
}

void ScreenManager::draw() {
  // Robot eyes screen is handled by RoboEyes library in main loop
  if (currentScreen == SCREEN_ROBOT_EYES) {
    return; // Don't draw anything, RoboEyes handles it
  }
  
  display->clearDisplay();
  
  switch(currentScreen) {
    case SCREEN_CLOCK:
      drawClock();
      break;
      
    case SCREEN_PRAYER_TIME:
      drawPrayerTime();
      break;
      
    case SCREEN_WEATHER:
      drawWeather();
      break;
      
    case SCREEN_SETTINGS:
      drawSettings();
      break;
      
    default:
      break;
  }
  
  display->display();
}

void ScreenManager::drawClock() {
  display->setTextSize(2);
  display->setTextColor(SSD1306_WHITE);
  
  if (timeSynced) {
    // Update time info
    time_t now;
    time(&now);
    localtime_r(&now, &timeInfo);
    
    char timeStr[9];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeInfo);
    
    // Center the time
    int16_t x1, y1;
    uint16_t w, h;
    display->getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);
    int x = (128 - w) / 2;
    int y = (64 - h) / 2;
    
    display->setCursor(x, y);
    display->print(timeStr);
    
    // Show date below
    display->setTextSize(1);
    char dateStr[12];
    strftime(dateStr, sizeof(dateStr), "%d/%m/%Y", &timeInfo);
    display->getTextBounds(dateStr, 0, 0, &x1, &y1, &w, &h);
    x = (128 - w) / 2;
    display->setCursor(x, y + h + 5);
    display->print(dateStr);
  } else {
    // Show "No Time Sync"
    display->setTextSize(1);
    display->setCursor(20, 28);
    display->print("No Time Sync");
    display->setCursor(15, 40);
    display->print("Connect WiFi");
  }
}

void ScreenManager::drawPrayerTime() {
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  
  display->setCursor(10, 5);
  display->print("Next Prayer:");
  
  if (nextPrayerName.length() > 0) {
    display->setTextSize(2);
    display->setCursor(10, 18);
    display->print(nextPrayerName);
    
    display->setTextSize(1);
    display->setCursor(10, 38);
    display->print("Time: ");
    display->print(nextPrayerTime);
    
    if (minutesUntilPrayer > 0) {
      display->setCursor(10, 48);
      display->print("In: ");
      display->print(minutesUntilPrayer);
      display->print(" min");
    }
  } else {
    display->setTextSize(1);
    display->setCursor(10, 30);
    display->print("No prayer data");
    display->setCursor(10, 40);
    display->print("Connect WiFi");
  }
}

void ScreenManager::drawWeather() {
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  
  if (weatherCached) {
    display->setCursor(5, 5);
    display->print("(Cached)");
  }
  
  if (temperature != 0.0 || weatherCondition.length() > 0) {
    display->setTextSize(3);
    display->setCursor(10, 20);
    display->print(temperature, 1);
    display->setTextSize(2);
    display->print("C");
    
    display->setTextSize(1);
    display->setCursor(10, 50);
    display->print(weatherCondition);
    
    if (weatherIcon.length() > 0) {
      display->setCursor(100, 25);
      display->setTextSize(2);
      display->print(weatherIcon);
    }
  } else {
    display->setTextSize(1);
    display->setCursor(10, 30);
    display->print("No weather data");
    display->setCursor(10, 40);
    display->print("Connect WiFi");
  }
}

void ScreenManager::drawSettings() {
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  
  // Settings page 0: WiFi Status
  if (settingsPage == 0) {
    display->setCursor(5, 5);
    display->print("WiFi Status");
    display->setCursor(5, 15);
    if (wifiSSID.length() > 0) {
      display->print("SSID: ");
      String ssid = wifiSSID;
      if (ssid.length() > 15) ssid = ssid.substring(0, 12) + "...";
      display->print(ssid);
    } else {
      display->print("SSID: Not connected");
    }
    display->setCursor(5, 25);
    if (wifiIP.length() > 0) {
      display->print("IP: ");
      display->print(wifiIP);
    } else {
      display->print("IP: N/A");
    }
    display->setCursor(5, 35);
    if (wifiRSSI != 0) {
      display->print("Signal: ");
      display->print(wifiRSSI);
      display->print(" dBm");
    } else {
      display->print("Signal: N/A");
    }
    display->setCursor(5, 45);
    display->print("BT: ");
    display->print(bluetoothEnabled ? "ON" : "OFF");
  }
  // Settings page 1: API Updates
  else if (settingsPage == 1) {
    display->setCursor(5, 5);
    display->print("Last Updates");
    display->setCursor(5, 15);
    display->print("Weather: ");
    if (lastWeatherUpdate.length() > 0) {
      display->print(lastWeatherUpdate);
    } else {
      display->print("Never");
    }
    display->setCursor(5, 25);
    display->print("Prayer: ");
    if (lastPrayerUpdate.length() > 0) {
      display->print(lastPrayerUpdate);
    } else {
      display->print("Never");
    }
    display->setCursor(5, 35);
    display->print("NTP: ");
    if (lastNTPUpdate.length() > 0) {
      display->print(lastNTPUpdate);
    } else {
      display->print("Never");
    }
  }
  // Settings page 2: Location
  else if (settingsPage == 2) {
    display->setCursor(5, 5);
    display->print("Location");
    display->setCursor(5, 15);
    display->print("Monastir, Tunisia");
    display->setCursor(5, 25);
    display->print("Lat: 35.7784");
    display->setCursor(5, 35);
    display->print("Lon: 10.8262");
  }
  // Settings page 3: System Info
  else if (settingsPage == 3) {
    display->setCursor(5, 5);
    display->print("System Info");
    display->setCursor(5, 15);
    display->print("Firmware: 1.0");
    
    // Uptime
    unsigned long uptime = millis() / 1000;
    unsigned long hours = uptime / 3600;
    unsigned long minutes = (uptime % 3600) / 60;
    display->setCursor(5, 25);
    display->print("Uptime: ");
    if (hours > 0) {
      display->print(hours);
      display->print("h ");
    }
    display->print(minutes);
    display->print("m");
    
    // Free heap
    display->setCursor(5, 35);
    display->print("Heap: ");
    display->print(ESP.getFreeHeap() / 1024);
    display->print(" KB");
  }
  
  // Page indicator
  display->setCursor(5, 55);
  display->print("Page ");
  display->print(settingsPage + 1);
  display->print("/4");
}

void ScreenManager::setTime(struct tm* timeInfo) {
  if (timeInfo != nullptr) {
    this->timeInfo = *timeInfo;
    timeSynced = true;
  }
}

void ScreenManager::setNextPrayer(String name, String time, int minutes) {
  nextPrayerName = name;
  nextPrayerTime = time;
  minutesUntilPrayer = minutes;
}

void ScreenManager::setWeather(float temp, String condition, String icon, bool cached) {
  temperature = temp;
  weatherCondition = condition;
  weatherIcon = icon;
  weatherCached = cached;
}

void ScreenManager::setWiFiInfo(String ssid, String ip, int rssi) {
  wifiSSID = ssid;
  wifiIP = ip;
  wifiRSSI = rssi;
}

void ScreenManager::nextSettingsPage() {
  settingsPage = (settingsPage + 1) % 4;
}

