/*
 * Mochi Robot - Main Program
 * Complete interaction system with emotions, touch reactions, and feeding
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "driver/i2s.h"
#include "mochi_face.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>

// Display setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Touch sensor
#define TOUCH_PIN 2

// I2S Audio pins
#define I2S_BCLK 4
#define I2S_LRC  5
#define I2S_DIN  6
#define I2S_SD   7

// Mochi face
MochiFace mochi(&display);

// State management
int touchCount = 0;
unsigned long lastTouchTime = 0;
int hungerPercent = 50; // Start at 50%
int energyPercent = 100; // Start at 100%
unsigned long lastHungerUpdate = 0;
unsigned long lastEnergyUpdate = 0;
unsigned long lastFeedTime = 0;
bool isFeeding = false;

// Random behavior system
unsigned long lastRandomAction = 0;
unsigned long randomActionInterval = 0;
int randomActionType = 0;

// Touch state
bool touchState = false;
bool lastTouchState = false;

// WiFi Access Point (default/fallback)
const char* ap_ssid = "Mochi-Robot";
const char* ap_password = "mochi123"; // Simple password for easy connection
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// WiFi Configuration Storage
Preferences preferences;
bool isConfigured = false;
String savedSSID = "";
String savedPassword = "";
bool wifiConnected = false;
IPAddress wifiIP;

// Web Server
WebServer server(80);
bool clientConnected = false;
unsigned long lastClientConnect = 0;

// Network Discovery
WiFiUDP udp;
const int DISCOVERY_PORT = 8888;
unsigned long lastDiscoveryPing = 0;
const unsigned long DISCOVERY_INTERVAL = 2000; // Send discovery ping every 2 seconds

// Forward declaration
void generateTone(int frequency, int duration);
void updateHunger();
void updateEnergy();
void handleTouch();
void handleFeeding();
void updateEmotionFromState();
void handleRandomBehavior();
void initWiFi();
void loadWiFiConfig();
void saveWiFiConfig(String ssid, String pass);
void connectToWiFi();
void startAccessPoint();
void handleRoot();
void handleStatus();
void handleWiFiConfig();
void handleWiFiConfigPost();
void handleNotFound();
void sendDiscoveryPing();
String getStatusJSON();
String getWiFiInfoJSON();

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
  
  // Initialize Touch
  Serial.println("Initializing Touch Sensor...");
  pinMode(TOUCH_PIN, INPUT);
  Serial.println("Touch: OK");
  
  // Initialize Audio
  Serial.println("Initializing Audio...");
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };
  
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRC,
    .data_out_num = I2S_DIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  
  esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.print("I2S driver install failed: ");
    Serial.println(err);
  } else {
    err = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (err != ESP_OK) {
      Serial.print("I2S pin config failed: ");
      Serial.println(err);
    }
    
    pinMode(I2S_SD, OUTPUT);
    digitalWrite(I2S_SD, HIGH);
    delay(10);
    
    Serial.println("Audio: OK");
  }
  
  // Set initial emotion to idle
  mochi.setNeutral();
  
  // Initialize random behavior system
  randomSeed(analogRead(0)); // Seed with noise
  lastRandomAction = millis();
  randomActionInterval = 3000 + random(7000); // First action in 3-10 seconds
  
  // Initialize WiFi (check for saved config or start AP)
  initWiFi();
  
  Serial.println("=== Mochi Robot Ready ===");
  Serial.println("Touch to interact, hunger system active");
  Serial.println("Random behaviors enabled!");
  Serial.println("Energy system active!");
  
  if (wifiConnected) {
    Serial.print("✅ Connected to WiFi: ");
    Serial.println(savedSSID);
    Serial.print("IP Address: ");
    Serial.println(wifiIP);
  } else {
    Serial.print("📡 Access Point Mode - SSID: ");
    Serial.println(ap_ssid);
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
  }
  Serial.println("Connect via app or visit http://192.168.4.1");
}

void loop() {
  static unsigned long lastFrame = 0;
  unsigned long now = millis();
  
  // Frame rate control (~30 FPS)
  if (now - lastFrame < 33) {
    return;
  }
  lastFrame = now;
  
  // Update hunger (increases over time)
  updateHunger();
  
  // Update energy (decreases over time, increases when sleeping)
  updateEnergy();
  
  // Update Mochi face animations
  mochi.update();
  
  // Handle random behaviors (like a real pet!)
  handleRandomBehavior();
  
  // Update emotion based on current state
  updateEmotionFromState();
  
  // Draw face with hunger, energy percentage, and WiFi status
  mochi.draw(hungerPercent, energyPercent, wifiConnected);
  
  // Handle web server requests
  server.handleClient();
  
  // Send discovery ping (UDP broadcast) when connected to WiFi
  if (wifiConnected && (millis() - lastDiscoveryPing > DISCOVERY_INTERVAL)) {
    sendDiscoveryPing();
    lastDiscoveryPing = millis();
  }
  
  // Check WiFi connection status periodically (if in STA mode)
  static unsigned long lastWiFiCheck = 0;
  if (wifiConnected && (millis() - lastWiFiCheck > 30000)) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("⚠️ WiFi disconnected, restarting AP...");
      wifiConnected = false;
      startAccessPoint();
    }
    lastWiFiCheck = millis();
  }
  
  // Reset connection flag after 10 seconds of no requests
  if (clientConnected && (millis() - lastClientConnect > 10000)) {
    clientConnected = false;
    Serial.println("📱 Client disconnected (timeout)");
  }
  
  // Read touch sensor
  touchState = digitalRead(TOUCH_PIN);
  
  // Detect touch press (edge detection)
  if (touchState == HIGH && lastTouchState == LOW) {
    handleTouch();
  }
  
  // Check for feeding (long press = feed)
  static unsigned long touchStartTime = 0;
  if (touchState == HIGH && lastTouchState == LOW) {
    touchStartTime = now; // Start of touch
  }
  if (touchState == HIGH && (now - touchStartTime) > 1500 && (now - lastFeedTime) > 3000) {
    // 1.5 second hold = feed (prevent spam)
    handleFeeding();
    touchStartTime = 0; // Reset
  }
  
  lastTouchState = touchState;
  
  // Decay touch count over time (reset after 10 seconds of no touch)
  if (now - lastTouchTime > 10000 && touchCount > 0) {
    touchCount = 0;
    Serial.println("Touch count reset");
  }
}

void updateHunger() {
  unsigned long now = millis();
  
  // Hunger increases by 1% every 10 seconds (faster for testing)
  if (now - lastHungerUpdate > 10000) {
    if (!isFeeding && hungerPercent < 100) {
      hungerPercent++;
      lastHungerUpdate = now;
      Serial.print("Hunger: ");
      Serial.print(hungerPercent);
      Serial.println("%");
    }
  }
}

void updateEnergy() {
  unsigned long now = millis();
  Emotion current = mochi.getEmotion();
  
  // Energy decreases by 1% every 60 seconds when awake
  // Energy increases when sleeping
  if (now - lastEnergyUpdate > 60000) {
    if (current == EMO_SLEEPING) {
      // Recover energy while sleeping (5% per minute)
      if (energyPercent < 100) {
        energyPercent = min(100, energyPercent + 5);
        Serial.print("Energy recovering: ");
        Serial.print(energyPercent);
        Serial.println("%");
      }
    } else {
      // Consume energy when awake (1% per minute)
      if (energyPercent > 0) {
        energyPercent = max(0, energyPercent - 1);
        Serial.print("Energy: ");
        Serial.print(energyPercent);
        Serial.println("%");
      }
    }
    lastEnergyUpdate = now;
  }
  
  // Auto-sleep when energy is very low
  if (energyPercent < 10 && current != EMO_SLEEPING) {
    mochi.setEmotion(EMO_SLEEPING, 0);
    Serial.println("😴 Energy too low - Going to sleep!");
  }
  
  // Get sleepy when energy is low
  if (energyPercent < 20 && energyPercent >= 10 && current != EMO_SLEEPING && current != EMO_SLEEPY) {
    if (current == EMO_IDLE || current == EMO_FULL) {
      mochi.setEmotion(EMO_SLEEPY, 0);
      Serial.println("😴 Low energy - Getting sleepy...");
    }
  }
}

void handleTouch() {
  touchCount++;
  lastTouchTime = millis();
  
  Serial.print("Touch #");
  Serial.println(touchCount);
  
  // Progressive touch reactions
  if (touchCount == 1) {
    // 1st touch → Pet Happy (purr)
    mochi.setEmotion(EMO_PET_HAPPY, 1500);
    generateTone(400, 200);
    Serial.println("😸 Pet Happy - Purr");
    
  } else if (touchCount == 2) {
    // 2nd touch → Pet Happy (lean in)
    mochi.setEmotion(EMO_PET_HAPPY, 1500);
    generateTone(450, 200);
    Serial.println("😸 Pet Happy - Lean in");
    
  } else if (touchCount == 3) {
    // 3rd touch → Pet Love (heart eyes)
    mochi.setEmotion(EMO_LOVE, 2000);
    generateTone(500, 250);
    Serial.println("😻 Pet Love - Heart eyes!");
    
  } else if (touchCount == 5) {
    // 5th touch → Pet Annoyed (look away)
    mochi.setEmotion(EMO_PET_ANNOYED, 2000);
    generateTone(300, 150);
    Serial.println("😾 Pet Annoyed - Look away");
    
  } else if (touchCount == 10) {
    // 10th touch → Pet Annoyed (angry swat)
    mochi.setEmotion(EMO_PET_ANNOYED, 2500);
    generateTone(250, 200);
    Serial.println("😾 Pet Annoyed - Angry swat!");
    
  } else if (touchCount >= 15) {
    // 15+ touches → Rage (stop spamming!)
    mochi.setEmotion(EMO_ANGRY, 3000);
    generateTone(200, 300);
    Serial.println("😠 RAGE - Stop spamming!");
  }
}

void handleFeeding() {
  if (isFeeding) return; // Prevent double feeding
  
  isFeeding = true;
  lastFeedTime = millis();
  
  Serial.print("Feeding! Hunger was: ");
  Serial.print(hungerPercent);
  Serial.println("%");
  
  // Feed response based on hunger level
  if (hungerPercent < 70) {
    // Feed @ <70% → Nom Nom (happy eating)
    mochi.setEmotion(EMO_EATING, 2000);
    generateTone(600, 300);
    hungerPercent = max(0, hungerPercent - 30);
    Serial.println("😋 Nom Nom - Happy eating!");
    
  } else if (hungerPercent >= 70 && hungerPercent < 90) {
    // Feed @ 70-90% → Stuffed (uncomfortable)
    mochi.setEmotion(EMO_FULL, 2500);
    generateTone(400, 200);
    hungerPercent = max(0, hungerPercent - 20);
    Serial.println("😌 Full - Stuffed!");
    
  } else if (hungerPercent >= 90) {
    // Feed @ >90% → Throw Up (sick)
    mochi.setEmotion(EMO_THROW_UP, 3000);
    generateTone(200, 400);
    hungerPercent = 50; // Reset after throwing up
    Serial.println("🤮 Throw Up - Too much!");
  }
  
  isFeeding = false;
}

void updateEmotionFromState() {
  // Don't override active emotions (unless they're done)
  Emotion current = mochi.getEmotion();
  unsigned long now = millis();
  
  // Don't override random behaviors or recent interactions
  // Give random behaviors time to play out (5 seconds)
  if ((now - lastRandomAction) < 5000 && current != EMO_IDLE && 
      current != EMO_HUNGRY && current != EMO_STARVING) {
    return; // Let random behavior finish
  }
  
  // If emotion is done and we're not in a special state, update based on hunger
  if (current == EMO_IDLE || (now - lastTouchTime > 3000 && touchCount < 15)) {
    
    // Don't override sleep state (energy takes priority)
  if (current == EMO_SLEEPING || current == EMO_SLEEPY) {
    return; // Let energy system handle sleep
  }
  
  // Update based on hunger percentage
    if (hungerPercent >= 90) {
      // 90-100% → Starving (critical)
      if (current != EMO_STARVING) {
        mochi.setEmotion(EMO_STARVING, 0);
        Serial.println("☠️ Starving - Critical!");
      }
      
    } else if (hungerPercent >= 70) {
      // 70-90% → Hungry (begging)
      if (current != EMO_HUNGRY) {
        mochi.setEmotion(EMO_HUNGRY, 0);
        Serial.println("🍽️ Hungry - Begging");
      }
      
    } else if (hungerPercent >= 50) {
      // 50-70% → Neutral
      if (current != EMO_IDLE && current != EMO_HAPPY && current != EMO_PET_HAPPY) {
        mochi.setEmotion(EMO_IDLE, 0);
      }
      
    } else if (hungerPercent >= 20) {
      // 20-50% → Idle (normal)
      if (current != EMO_IDLE && current != EMO_HAPPY && current != EMO_PET_HAPPY) {
        mochi.setEmotion(EMO_IDLE, 0);
      }
      
    } else {
      // 0-20% → Full/Satisfied
      if (current != EMO_FULL) {
        mochi.setEmotion(EMO_FULL, 0);
        Serial.println("😌 Full - Satisfied");
      }
    }
  }
  
  // Special: If neglected (no touch for 60 seconds and hungry), show crying
  if (now - lastTouchTime > 60000 && hungerPercent > 70 && current != EMO_CRYING) {
    mochi.setEmotion(EMO_CRYING, 0);
    Serial.println("😢 Crying - Neglected");
  }
}

void handleRandomBehavior() {
  unsigned long now = millis();
  Emotion current = mochi.getEmotion();
  
  // Don't interrupt active interactions or feeding
  if (isFeeding || (now - lastTouchTime) < 2000) {
    return;
  }
  
  // Check if it's time for a random action
  if (now - lastRandomAction > randomActionInterval) {
    lastRandomAction = now;
    
    // Random interval for next action (5-15 seconds)
    randomActionInterval = 5000 + random(10000);
    
    // Random action type (0-100)
    randomActionType = random(100);
    
    // Only do random actions if in idle or neutral states
    if (current == EMO_IDLE || current == EMO_FULL || 
        (current == EMO_HUNGRY && hungerPercent < 80)) {
      
      if (randomActionType < 15) {
        // 15% chance - Random happy moment
        mochi.setEmotion(EMO_HAPPY, 2000 + random(2000));
        generateTone(500 + random(200), 150);
        Serial.println("🎲 Random: Happy moment!");
        
      } else if (randomActionType < 25) {
        // 10% chance - Yawn/stretch (sleepy)
        mochi.setEmotion(EMO_SLEEPY, 2500);
        generateTone(300, 300);
        Serial.println("🎲 Random: Yawn...");
        
      } else if (randomActionType < 35) {
        // 10% chance - Look around (surprised)
        mochi.setEmotion(EMO_SURPRISED, 1500);
        generateTone(600, 100);
        Serial.println("🎲 Random: What's that?");
        
      } else if (randomActionType < 42) {
        // 7% chance - Thinking
        mochi.setEmotion(EMO_THINKING, 3000);
        Serial.println("🎲 Random: Hmm...");
        
      } else if (randomActionType < 50) {
        // 8% chance - Playful (laughing)
        mochi.setEmotion(EMO_LAUGHING, 2000);
        generateTone(400 + random(300), 200);
        Serial.println("🎲 Random: Hehe!");
        
      } else if (randomActionType < 55 && hungerPercent < 50) {
        // 5% chance - Content purr (only when not hungry)
        mochi.setEmotion(EMO_PET_HAPPY, 1500);
        generateTone(350, 250);
        Serial.println("🎲 Random: Purr...");
        
      } else if (randomActionType < 60) {
        // 5% chance - Blink and look away
        mochi.setEmotion(EMO_IDLE, 0);
        Serial.println("🎲 Random: Just looking around");
        
      } else if (randomActionType < 70 && hungerPercent > 60) {
        // 10% chance - Complain if getting hungry
        mochi.setEmotion(EMO_SAD, 2000);
        generateTone(300, 200);
        Serial.println("🎲 Random: Getting hungry...");
        
      } else if (randomActionType < 75) {
        // 5% chance - Random sound
        generateTone(400 + random(400), 100 + random(100));
        Serial.println("🎲 Random: Random noise");
        
      } else if (randomActionType < 85) {
        // 10% chance - Quick emotion flash
        int quickEmotion = random(5);
        switch(quickEmotion) {
          case 0: mochi.setEmotion(EMO_HAPPY, 800); break;
          case 1: mochi.setEmotion(EMO_SURPRISED, 800); break;
          case 2: mochi.setEmotion(EMO_LOVE, 800); break;
          case 3: mochi.setEmotion(EMO_THINKING, 800); break;
          default: mochi.setEmotion(EMO_LAUGHING, 800); break;
        }
        Serial.println("🎲 Random: Quick emotion flash");
        
      } else {
        // 15% chance - Do nothing (just stay idle)
        // This makes it feel more natural - not always doing something
        Serial.println("🎲 Random: Just chilling...");
      }
    }
  }
}

void generateTone(int frequency, int duration) {
  int sampleRate = 44100;
  int samples = (sampleRate * duration) / 1000;
  int16_t *audioBuffer = (int16_t*)malloc(samples * sizeof(int16_t));
  
  if (audioBuffer == NULL) {
    return;
  }
  
  // Generate sine wave
  for (int i = 0; i < samples; i++) {
    float sample = sin(2.0 * PI * frequency * i / sampleRate);
    audioBuffer[i] = (int16_t)(sample * 16383);
  }
  
  // Write to I2S
  size_t bytesWritten = 0;
  i2s_write(I2S_NUM_0, audioBuffer, samples * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
  
  free(audioBuffer);
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
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/wifi/info", HTTP_GET, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", getWiFiInfoJSON());
  });
  server.on("/wifi/config", HTTP_POST, handleWiFiConfigPost);
  
  // Handle OPTIONS for CORS preflight
  server.on("/status", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(200, "text/plain", "");
  });
  
  server.on("/wifi/config", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(200, "text/plain", "");
  });
  
  server.onNotFound(handleNotFound);
  
  // Start web server
  server.begin();
  Serial.println("✅ Web server started!");
}

void loadWiFiConfig() {
  preferences.begin("mochi", false);
  savedSSID = preferences.getString("ssid", "");
  savedPassword = preferences.getString("pass", "");
  isConfigured = (savedSSID.length() > 0);
  preferences.end();
  
  if (isConfigured) {
    Serial.print("📋 Loaded WiFi config: ");
    Serial.println(savedSSID);
  } else {
    Serial.println("📋 No saved WiFi configuration found");
  }
}

void saveWiFiConfig(String ssid, String pass) {
  preferences.begin("mochi", false);
  preferences.putString("ssid", ssid);
  preferences.putString("pass", pass);
  preferences.end();
  
  savedSSID = ssid;
  savedPassword = pass;
  isConfigured = true;
  
  Serial.print("💾 Saved WiFi config: ");
  Serial.println(ssid);
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
    wifiIP = WiFi.localIP();
    Serial.println("✅ WiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(wifiIP);
    
    // Start mDNS
    if (MDNS.begin("mochi-robot")) {
      Serial.println("✅ mDNS started! Access at http://mochi-robot.local");
      MDNS.addService("http", "tcp", 80);
    } else {
      Serial.println("❌ mDNS failed to start");
    }
    
    // Start UDP for discovery broadcasts
    udp.begin(DISCOVERY_PORT);
    Serial.println("✅ UDP discovery started on port 8888");
  } else {
    wifiConnected = false;
    Serial.println("❌ WiFi connection failed!");
    Serial.println("Falling back to Access Point mode...");
  }
}

void startAccessPoint() {
  // Configure WiFi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  
  // Start Access Point
  bool apStarted = WiFi.softAP(ap_ssid, ap_password);
  
  if (!apStarted) {
    Serial.println("❌ WiFi AP failed to start!");
    return;
  }
  
  Serial.println("✅ WiFi AP started!");
  Serial.print("SSID: ");
  Serial.println(ap_ssid);
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void handleWiFiConfigPost() {
  lastClientConnect = millis();
  
  // Add CORS headers
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    Serial.print("📥 Received WiFi config: ");
    Serial.println(body);
    
    // Parse JSON (simple parsing)
    String ssid = "";
    String password = "";
    
    int ssidStart = body.indexOf("\"ssid\":\"");
    int ssidEnd = body.indexOf("\"", ssidStart + 8);
    if (ssidStart >= 0 && ssidEnd > ssidStart) {
      ssid = body.substring(ssidStart + 8, ssidEnd);
    }
    
    int passStart = body.indexOf("\"password\":\"");
    int passEnd = body.indexOf("\"", passStart + 12);
    if (passStart >= 0 && passEnd > passStart) {
      password = body.substring(passStart + 12, passEnd);
    }
    
    if (ssid.length() > 0 && password.length() > 0) {
      // Save configuration
      saveWiFiConfig(ssid, password);
      
      // Try to connect
      connectToWiFi();
      
      if (wifiConnected) {
        // Stop AP and restart server
        WiFi.softAPdisconnect(true);
        server.send(200, "application/json", "{\"success\":true,\"message\":\"WiFi configured and connected!\",\"ip\":\"" + wifiIP.toString() + "\"}");
        Serial.println("✅ WiFi configured successfully!");
      } else {
        // Keep AP running but save config for next reboot
        server.send(200, "application/json", "{\"success\":false,\"message\":\"Failed to connect to WiFi. Config saved for next reboot.\"}");
        Serial.println("⚠️ WiFi config saved but connection failed");
      }
    } else {
      server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid request\"}");
    }
  } else {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"No data received\"}");
  }
}

String getWiFiInfoJSON() {
  String json = "{";
  json += "\"configured\":" + String(isConfigured ? "true" : "false") + ",";
  json += "\"connected\":" + String(wifiConnected ? "true" : "false") + ",";
  json += "\"mode\":\"" + String(wifiConnected ? "STA" : "AP") + "\",";
  if (wifiConnected) {
    json += "\"ssid\":\"" + savedSSID + "\",";
    json += "\"ip\":\"" + wifiIP.toString() + "\",";
    json += "\"mac\":\"" + WiFi.macAddress() + "\"";
  } else {
    json += "\"ap_ssid\":\"" + String(ap_ssid) + "\",";
    json += "\"ap_ip\":\"" + WiFi.softAPIP().toString() + "\"";
  }
  json += "}";
  return json;
}

void sendDiscoveryPing() {
  // Send UDP broadcast packet for discovery
  IPAddress broadcastIP = WiFi.localIP();
  broadcastIP[3] = 255; // Broadcast to .255
  
  String discoveryMsg = "MOCHI_ROBOT:" + wifiIP.toString() + ":" + WiFi.macAddress();
  
  udp.beginPacket(broadcastIP, DISCOVERY_PORT);
  udp.write((const uint8_t*)discoveryMsg.c_str(), discoveryMsg.length());
  udp.endPacket();
  
  // Also send to common broadcast addresses
  IPAddress broadcast255(255, 255, 255, 255);
  udp.beginPacket(broadcast255, DISCOVERY_PORT);
  udp.write((const uint8_t*)discoveryMsg.c_str(), discoveryMsg.length());
  udp.endPacket();
}

void handleRoot() {
  // Track client connection
  if (!clientConnected) {
    clientConnected = true;
    Serial.println("📱 Client connected via WiFi!");
    
    // Play 3 beeps to indicate connection
    generateTone(600, 200);
    delay(100);
    generateTone(600, 200);
    delay(100);
    generateTone(600, 200);
    Serial.println("🔔 Connection beeps played");
  }
  lastClientConnect = millis();
  
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>Mochi Robot</title>";
  html += "<style>body{font-family:Arial;text-align:center;background:#1a1a1a;color:#fff;padding:20px;}";
  html += ".status{background:#2a2a2a;padding:20px;border-radius:10px;margin:10px;}";
  html += ".bar{background:#333;height:30px;border-radius:15px;overflow:hidden;margin:10px 0;}";
  html += ".fill{height:100%;transition:width 0.3s;border-radius:15px;}";
  html += ".hunger{background:#ff6b6b;}";
  html += ".energy{background:#4ecdc4;}";
  html += "button{background:#4ecdc4;color:#fff;border:none;padding:15px 30px;border-radius:5px;font-size:16px;margin:10px;cursor:pointer;}";
  html += "button:hover{background:#45b8b0;}</style></head><body>";
  html += "<h1>🤖 Mochi Robot</h1>";
  html += "<div id='status'></div>";
  html += "<button onclick='location.reload()'>Refresh</button>";
  html += "<script>setInterval(()=>fetch('/status').then(r=>r.json()).then(d=>{";
  html += "document.getElementById('status').innerHTML=";
  html += "'<div class=status><h2>'+d.emotion+'</h2>";
  html += "<p>Hunger: '+d.hunger+'%</p><div class=bar><div class=\"fill hunger\" style=\"width:'+d.hunger+'%\"></div></div>";
  html += "<p>Energy: '+d.energy+'%</p><div class=bar><div class=\"fill energy\" style=\"width:'+d.energy+'%\"></div></div>";
  html += "<p>Touches: '+d.touchCount+'</p></div>';";
  html += "}),1000);</script></body></html>";
  server.send(200, "text/html", html);
}

void handleStatus() {
  lastClientConnect = millis();
  
  // Track client connection
  if (!clientConnected) {
    clientConnected = true;
    Serial.println("📱 Client connected via WiFi!");
    
    // Play 3 beeps to indicate connection
    generateTone(600, 200);
    delay(100);
    generateTone(600, 200);
    delay(100);
    generateTone(600, 200);
    Serial.println("🔔 Connection beeps played");
  }
  
  // Add CORS headers for mobile app
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  
  String json = getStatusJSON();
  Serial.print("📤 Sending status: ");
  Serial.println(json);
  
  server.send(200, "application/json", json);
}

void handleNotFound() {
  server.send(404, "text/plain", "Not Found");
}

String getStatusJSON() {
  // Get emotion name
  const char* emotionName = "IDLE";
  Emotion currentEmo = mochi.getEmotion();
  switch(currentEmo) {
    case EMO_IDLE: emotionName = "IDLE"; break;
    case EMO_HAPPY: emotionName = "HAPPY"; break;
    case EMO_SAD: emotionName = "SAD"; break;
    case EMO_CRYING: emotionName = "CRYING"; break;
    case EMO_ANGRY: emotionName = "ANGRY"; break;
    case EMO_SURPRISED: emotionName = "SURPRISED"; break;
    case EMO_LOVE: emotionName = "LOVE"; break;
    case EMO_SLEEPY: emotionName = "SLEEPY"; break;
    case EMO_SLEEPING: emotionName = "SLEEPING"; break;
    case EMO_THINKING: emotionName = "THINKING"; break;
    case EMO_LAUGHING: emotionName = "LAUGHING"; break;
    case EMO_PET_HAPPY: emotionName = "PET HAPPY"; break;
    case EMO_PET_LOVE: emotionName = "PET LOVE"; break;
    case EMO_PET_ANNOYED: emotionName = "PET ANNOYED"; break;
    case EMO_EATING: emotionName = "EATING"; break;
    case EMO_HUNGRY: emotionName = "HUNGRY"; break;
    case EMO_FULL: emotionName = "FULL"; break;
    case EMO_THROW_UP: emotionName = "THROWING UP"; break;
    case EMO_STARVING: emotionName = "STARVING"; break;
    case EMO_SICK: emotionName = "SICK"; break;
    default: emotionName = "IDLE"; break;
  }

  // Create JSON status
  String jsonString = "{";
  jsonString += "\"emotion\":\"" + String(emotionName) + "\",";
  jsonString += "\"hunger\":" + String(hungerPercent) + ",";
  jsonString += "\"energy\":" + String(energyPercent) + ",";
  jsonString += "\"touchCount\":" + String(touchCount);
  jsonString += "}";
  
  return jsonString;
}
