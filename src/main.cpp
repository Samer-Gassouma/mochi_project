/*
 * Mochi Robot - Main Program
 * ESP32-C3 powered robot with OLED, touch sensor, Bluetooth, WiFi, and emotional behavior
 * Based on specification v1.0
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <time.h>
// Include ArduinoJson explicitly first to avoid macro conflicts
#include <ArduinoJson.h>
// Now include RoboEyes (which defines N and E macros)
#include "../RoboEyes/src/FluxGarage_RoboEyes.h"
#include "screen_manager.h"
#include "touch_handler.h"
#include "emotion_manager.h"
#include "weather_api.h"
#include "prayer_api.h"
#include "display_brightness.h"
#include "ble_setup.h"

// Display setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Touch sensor
#define TOUCH_PIN 2

// Buzzer (passive or active piezo) on GPIO 4
#define BUZZER_PIN 4
#define BUZZER_CHANNEL 0

// RoboEyes instance
RoboEyes<Adafruit_SSD1306> roboEyes(display);

// Preferences for NVS storage
Preferences preferences;

// Manager instances
ScreenManager screenManager(&display);
TouchHandler touchHandler(TOUCH_PIN);
EmotionManager emotionManager(&roboEyes);
WeatherAPI weatherAPI(&preferences);
PrayerAPI prayerAPI(&preferences);
DisplayBrightness displayBrightness(&display);
BleSetup bleSetup(&preferences);

// State management
bool wifiConnected = false;
bool isSleeping = false;
unsigned long lastInteractionTime = 0;
unsigned long sleepTimeout = 300000; // 5 minutes default

// WiFi Configuration Storage
bool isConfigured = false;
String savedSSID = "";
String savedPassword = "";

// BLE setup data
SetupData setupData;

// API data
WeatherData currentWeather;
PrayerData currentPrayer;

// Forward declarations
void generateTone(int frequency, int duration);
void purrSound(); // Purr sound when touched
void initWiFi();
void loadWiFiConfig();
void connectToWiFi();
void startAccessPoint();
void handleTouchEvents();
void updateSleepState();
void initNTP();

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== Mochi Robot Starting ===");
  
  // Initialize Display
  Serial.println("Initializing Display...");
  Wire.begin(8, 9);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("Display FAILED!");
    for(;;);
  }
  display.clearDisplay();
  display.display();
  Serial.println("Display: OK");
  
  // Initialize RoboEyes
  Serial.println("Initializing RoboEyes...");
  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 50); // 50 FPS
  roboEyes.setDisplayColors(0, 1); // Black background, white eyes
  roboEyes.setAutoblinker(ON, 3, 2); // Auto blink every 3-5 seconds
  roboEyes.setIdleMode(ON, 5, 3); // Idle mode: look around every 5-8 seconds
  roboEyes.setMood(DEFAULT);
  Serial.println("RoboEyes: OK");
  
  // Initialize Touch
  Serial.println("Initializing Touch Sensor...");
  Serial.println("Touch: OK");
  
  // Initialize Buzzer (PWM via LEDC)
  Serial.println("Initializing Buzzer...");
  ledcSetup(BUZZER_CHANNEL, 2000, 10); // 2 kHz default, 10-bit resolution
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  ledcWriteTone(BUZZER_CHANNEL, 0); // ensure silent
  Serial.println("Buzzer: OK");
  
  // Initialize BLE for setup
  Serial.println("Initializing BLE setup...");
  if (bleSetup.begin()) {
    Serial.println("✅ BLE advertising: Mochi-Robot-Setup");
    screenManager.setBluetoothEnabled(true);
  } else {
    Serial.println("⚠️ BLE init failed, continuing without setup mode");
    screenManager.setBluetoothEnabled(false);
  }
  
  // Load setup data from NVS
  if (bleSetup.getSetupData(&setupData)) {
    if (setupData.wifiSSID.length() > 0) {
      savedSSID = setupData.wifiSSID;
      savedPassword = setupData.wifiPassword;
      isConfigured = true;
    }
    if (setupData.weatherAPIKey.length() > 0) {
      weatherAPI.setAPIKey(setupData.weatherAPIKey);
    }
    // Location is hardcoded to Monastir for now, but can be overridden
    if (setupData.latitude != 0.0 && setupData.longitude != 0.0) {
      weatherAPI.setLocation(setupData.latitude, setupData.longitude);
      prayerAPI.setLocation(setupData.latitude, setupData.longitude);
      Serial.print("📍 Using saved location: ");
      Serial.print(setupData.latitude, 6);
      Serial.print(", ");
      Serial.println(setupData.longitude, 6);
    }
  }
  
  // Initialize WiFi
  initWiFi();
  
  // Initialize NTP if WiFi connected
  if (wifiConnected) {
    initNTP();
  }
  
  // Load cached weather and prayer data
  weatherAPI.loadCachedWeather(&currentWeather);
  if (currentWeather.temperature != 0.0 || currentWeather.condition.length() > 0) {
    screenManager.setWeather(currentWeather.temperature, currentWeather.condition, 
                           currentWeather.icon, currentWeather.cached);
  }
  
  prayerAPI.loadCachedPrayerTimes(&currentPrayer);
  prayerAPI.updateNextPrayer(&currentPrayer);
  if (currentPrayer.nextPrayerName.length() > 0) {
    screenManager.setNextPrayer(currentPrayer.nextPrayerName, currentPrayer.nextPrayerTime,
                               currentPrayer.minutesUntilNext);
  }
  
  // Initialize random seed for random emotions
  randomSeed(analogRead(A0) + millis());
  
  // Set initial emotion
  emotionManager.setNeutral();
  emotionManager.setOnline(wifiConnected);
  emotionManager.enableRandomEmotions(true); // Enable random emotions
  
  // Default location: Monastir, Tunisia (if not set via Bluetooth)
  if (setupData.latitude == 0.0 || setupData.longitude == 0.0) {
    weatherAPI.setLocation(35.7784, 10.8262);
    prayerAPI.setLocation(35.7784, 10.8262);
    Serial.println("📍 Location set to: Monastir, Tunisia (35.7784, 10.8262)");
  }
  
  // Try initial API fetch if WiFi connected
  if (wifiConnected) {
    // Note: Weather API requires API key (will be set via Bluetooth)
    // Prayer API is free and doesn't require a key
    Serial.println("🕌 Fetching initial prayer times...");
    if (prayerAPI.fetchPrayerTimes(&currentPrayer)) {
      screenManager.setNextPrayer(currentPrayer.nextPrayerName, currentPrayer.nextPrayerTime,
                                 currentPrayer.minutesUntilNext);
    }
    
    // Weather will be fetched once API key is set via Bluetooth
    Serial.println("🌤️ Weather API key needed (set via Bluetooth)");
  }
  
  // Play startup beep
  generateTone(600, 200);
  delay(100);
  generateTone(700, 200);
  
  Serial.println("=== Mochi Robot Ready ===");
  Serial.println("Touch to interact");
  Serial.println("Single tap: Next screen");
  Serial.println("Double tap: Special animation");
  Serial.println("Long press: Settings");
  
  if (wifiConnected) {
    Serial.print("✅ Connected to WiFi: ");
    Serial.println(savedSSID);
  } else {
    Serial.println("📡 WiFi not connected - Offline mode");
  }
}

void loop() {
  unsigned long now = millis();
  
  // Update touch handler
  touchHandler.update();
  
  // Handle touch events
  handleTouchEvents();
  
  // Update sleep state
  updateSleepState();
  
  // Update emotion manager
  emotionManager.update();
  
  // Update display brightness (for dimming animation)
  displayBrightness.update();
  
  // Check for new setup data from BLE and reconnect WiFi if needed
  static unsigned long lastBTCheck = 0;
  if (bleSetup.getIsEnabled() && (now - lastBTCheck > 2000)) {
    if (bleSetup.getSetupData(&setupData)) {
      if (setupData.wifiSSID.length() > 0 && setupData.wifiSSID != savedSSID) {
        Serial.println("📡 New WiFi credentials received via BLE, reconnecting...");
        savedSSID = setupData.wifiSSID;
        savedPassword = setupData.wifiPassword;
        isConfigured = true;
        WiFi.disconnect();
        delay(500);
        connectToWiFi();
      }
      if (setupData.weatherAPIKey.length() > 0) {
        weatherAPI.setAPIKey(setupData.weatherAPIKey);
      }
      if (setupData.latitude != 0.0 && setupData.longitude != 0.0) {
        weatherAPI.setLocation(setupData.latitude, setupData.longitude);
        prayerAPI.setLocation(setupData.latitude, setupData.longitude);
      }
    }
    lastBTCheck = now;
  }
  
  // Update RoboEyes (only on robot eyes screen and when awake)
  if (screenManager.getCurrentScreen() == SCREEN_ROBOT_EYES && !isSleeping) {
    roboEyes.update();
  } else if (!isSleeping) {
    // Update other screens (only when awake)
    screenManager.update();
  }
  
  // Bluetooth setup is handled in bluetoothSetup.update() above
  
  // Check WiFi connection status periodically
  static unsigned long lastWiFiCheck = 0;
  if (wifiConnected && (now - lastWiFiCheck > 30000)) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("⚠️ WiFi disconnected");
      wifiConnected = false;
      emotionManager.setOnline(false);
      startAccessPoint();
    }
    lastWiFiCheck = now;
  }
  
  // Update NTP time periodically (every hour)
  static unsigned long lastNTPUpdate = 0;
  if (wifiConnected && (now - lastNTPUpdate > 3600000)) {
    struct tm timeInfo;
    if (getLocalTime(&timeInfo)) {
      screenManager.setTime(&timeInfo);
      screenManager.setTimeSynced(true);
      Serial.println("🕐 NTP time updated");
    }
    lastNTPUpdate = now;
  }
  
  // Update weather data (every 30 minutes)
  static unsigned long lastWeatherUpdate = 0;
  if (wifiConnected && weatherAPI.needsUpdate() && (now - lastWeatherUpdate > 1800000)) {
    Serial.println("🌤️ Updating weather...");
    if (weatherAPI.fetchWeather(&currentWeather)) {
      screenManager.setWeather(currentWeather.temperature, currentWeather.condition,
                              currentWeather.icon, currentWeather.cached);
      Serial.println("✅ Weather updated");
    } else {
      // Load cached weather if fetch failed
      if (weatherAPI.loadCachedWeather(&currentWeather)) {
        screenManager.setWeather(currentWeather.temperature, currentWeather.condition,
                                currentWeather.icon, true);
      }
    }
    lastWeatherUpdate = now;
  }
  
  // Update prayer times (every hour)
  static unsigned long lastPrayerUpdate = 0;
  if (wifiConnected && prayerAPI.needsUpdate() && (now - lastPrayerUpdate > 3600000)) {
    Serial.println("🕌 Updating prayer times...");
    if (prayerAPI.fetchPrayerTimes(&currentPrayer)) {
      screenManager.setNextPrayer(currentPrayer.nextPrayerName, currentPrayer.nextPrayerTime,
                                 currentPrayer.minutesUntilNext);
      Serial.println("✅ Prayer times updated");
    } else {
      // Load cached prayer times if fetch failed
      if (prayerAPI.loadCachedPrayerTimes(&currentPrayer)) {
        prayerAPI.updateNextPrayer(&currentPrayer);
        screenManager.setNextPrayer(currentPrayer.nextPrayerName, currentPrayer.nextPrayerTime,
                                   currentPrayer.minutesUntilNext);
      }
    }
    lastPrayerUpdate = now;
  }
  
  // Update next prayer calculation every minute (for countdown)
  static unsigned long lastPrayerCalc = 0;
  if (now - lastPrayerCalc > 60000) {
    prayerAPI.updateNextPrayer(&currentPrayer);
    screenManager.setNextPrayer(currentPrayer.nextPrayerName, currentPrayer.nextPrayerTime,
                               currentPrayer.minutesUntilNext);
    lastPrayerCalc = now;
  }
  
  // Update last update times in settings (every minute)
  static unsigned long lastUpdateTimeDisplay = 0;
  if (now - lastUpdateTimeDisplay > 60000) {
    struct tm timeInfo;
    if (getLocalTime(&timeInfo)) {
      char timeStr[6];
      strftime(timeStr, sizeof(timeStr), "%H:%M", &timeInfo);
      
      if (currentWeather.lastUpdate > 0) {
        screenManager.setLastWeatherUpdate(timeStr);
      }
      if (currentPrayer.lastUpdate > 0) {
        screenManager.setLastPrayerUpdate(timeStr);
      }
      // NTP is always synced if we have time
      screenManager.setLastNTPUpdate(timeStr);
    }
    lastUpdateTimeDisplay = now;
  }
  
  // Update WiFi info periodically
  if (wifiConnected && (now - lastWiFiCheck > 30000)) {
    screenManager.setWiFiInfo(WiFi.SSID(), WiFi.localIP().toString(), WiFi.RSSI());
  }
  
  // Update Bluetooth status in settings (BLE)
  static unsigned long lastBTStatusUpdate = 0;
  if (now - lastBTStatusUpdate > 5000) {
    screenManager.setBluetoothEnabled(bleSetup.getIsEnabled());
    lastBTStatusUpdate = now;
  }
}

void handleTouchEvents() {
  TouchEvent event = touchHandler.getEvent();
  
  if (event == TOUCH_NONE) {
    return;
  }
  
  // Wake up if sleeping
  if (isSleeping) {
    isSleeping = false;
    roboEyes.open();
    displayBrightness.brighten(1000); // Brighten over 1 second
    generateTone(700, 200);
    delay(100);
    generateTone(800, 200);
    Serial.println("😴 Waking up...");
    return;
  }
  
  lastInteractionTime = millis();
  emotionManager.setInteracting(true);
  
  // Play purr sound on any touch (like a cat!)
  purrSound();
  
  // Handle settings screen navigation differently
  if (screenManager.getCurrentScreen() == SCREEN_SETTINGS) {
    switch(event) {
      case TOUCH_SINGLE_TAP:
        screenManager.nextSettingsPage();
        generateTone(400, 150);
        Serial.println("👆 Settings - Next page");
        break;
        
      case TOUCH_LONG_PRESS:
        screenManager.setScreen(SCREEN_ROBOT_EYES);
        generateTone(300, 200);
        Serial.println("👆 Long press - Exit settings");
        break;
        
      default:
        break;
    }
  } else {
    // Normal screen navigation
    switch(event) {
      case TOUCH_SINGLE_TAP:
        screenManager.nextScreen();
        generateTone(400, 200); // Click sound
        Serial.println("👆 Single tap - Next screen");
        break;
        
      case TOUCH_DOUBLE_TAP:
        emotionManager.setExcited();
        generateTone(500, 150);
        delay(50);
        generateTone(600, 150);
        Serial.println("👆👆 Double tap - Excited!");
        break;
        
      case TOUCH_LONG_PRESS:
        screenManager.setScreen(SCREEN_SETTINGS);
        generateTone(300, 300);
        Serial.println("👆 Long press - Settings");
        break;
        
      default:
        break;
    }
  }
  
  // Update interaction count for emotion
  static int interactionCount = 0;
  interactionCount++;
  emotionManager.setInteractionCount(interactionCount);
}

void updateSleepState() {
  unsigned long now = millis();
  
  // Check if should sleep
  if (!isSleeping && (now - lastInteractionTime) > sleepTimeout) {
    isSleeping = true;
    roboEyes.close();
    displayBrightness.dim(2000); // Dim over 2 seconds
    generateTone(400, 300); // Sleep beep
    Serial.println("😴 Going to sleep...");
    // TODO: Enter light sleep mode (esp_sleep)
  }
  
  // Check if should wake (handled in touch events, but ensure brightness is restored)
  if (isSleeping && displayBrightness.getIsDimmed()) {
    // Keep dimmed while sleeping
  }
}

void generateTone(int frequency, int duration) {
  // Simple square-wave tone using LEDC PWM
  ledcWriteTone(BUZZER_CHANNEL, frequency);
  delay(duration);
  ledcWriteTone(BUZZER_CHANNEL, 0);
}

void purrSound() {
  // Purr: modulated low-frequency tone (human-hearable range ~150-220 Hz)
  const int duration = 300; // ms
  const int steps = 30;     // modulation steps
  const int baseFreq = 180;
  const int modDepth = 40;  // +/- range
  const int modFreqMs = duration / steps;
  
  for (int i = 0; i < steps; i++) {
    float phase = (float)i / steps;
    float freq = baseFreq + modDepth * sinf(2.0f * PI * phase);
    ledcWriteTone(BUZZER_CHANNEL, (uint32_t)freq);
    delay(modFreqMs);
  }
  ledcWriteTone(BUZZER_CHANNEL, 0);
}

void initWiFi() {
  Serial.println("Initializing WiFi...");
  
  // Load saved WiFi configuration
  loadWiFiConfig();
  
  // If we have saved credentials, try to connect
  if (isConfigured && savedSSID.length() > 0) {
    Serial.println("📡 Found saved WiFi credentials, attempting to connect...");
    connectToWiFi();
  }
  
  // If connection failed or no config, start Access Point
  if (!wifiConnected) {
    Serial.println("📡 Starting Access Point mode...");
    startAccessPoint();
  }
  
  // Bluetooth setup replaces web server
  Serial.println("✅ WiFi initialization complete");
}

void loadWiFiConfig() {
  preferences.begin("mochi", false);
  
  // Default WiFi credentials (can be overridden via Bluetooth)
  String defaultSSID = "Ooredoo-320258";
  String defaultPassword = "Lost2409@root_kali";
  
  savedSSID = preferences.getString("ssid", defaultSSID);
  savedPassword = preferences.getString("pass", defaultPassword);
  
  // If using defaults, mark as configured
  if (savedSSID == defaultSSID) {
    isConfigured = true;
    Serial.println("📋 Using default WiFi credentials");
  } else {
    isConfigured = (savedSSID.length() > 0);
  }
  
  preferences.end();
  
  if (isConfigured) {
    Serial.print("📋 WiFi SSID: ");
    Serial.println(savedSSID);
  } else {
    Serial.println("📋 No WiFi configuration found");
  }
}

void connectToWiFi() {
  Serial.print("🔌 Connecting to WiFi: ");
  Serial.println(savedSSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
  
  // Wait for connection (max 15 seconds)
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("✅ WiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    // Update WiFi info in settings
    screenManager.setWiFiInfo(WiFi.SSID(), WiFi.localIP().toString(), WiFi.RSSI());
  } else {
    wifiConnected = false;
    Serial.println("❌ WiFi connection failed!");
    screenManager.setWiFiInfo("", "", 0);
  }
}

void startAccessPoint() {
  const char* ap_ssid = "Mochi-Robot";
  const char* ap_password = "mochi123";
  IPAddress local_IP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ap_ssid, ap_password);
  
  Serial.println("✅ WiFi AP started!");
  Serial.print("SSID: ");
  Serial.println(ap_ssid);
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void initNTP() {
  Serial.println("Initializing NTP...");
  
  // Set timezone (adjust as needed)
  setenv("TZ", "UTC", 1);
  tzset();
  
  configTime(0, 0, "pool.ntp.org", "time.google.com");
  
  // Wait for time sync
  struct tm timeInfo;
  int attempts = 0;
  while (!getLocalTime(&timeInfo) && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  
  if (attempts < 20) {
    Serial.println("✅ NTP time synchronized!");
    screenManager.setTime(&timeInfo);
    screenManager.setTimeSynced(true);
    
    // Print current time
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeInfo);
    Serial.print("Current time: ");
    Serial.println(timeStr);
  } else {
    Serial.println("❌ NTP time sync failed");
    screenManager.setTimeSynced(false);
  }
}
