/*
 * Mochi Robot - Combined Component Test
 * Tests all components together: Display, Touch, and Audio
 * 
 * WIRING CONNECTIONS:
 * 
 * SSD1306 Display (already connected):
 *   VCC -> ESP32-C3 3.3V
 *   GND -> ESP32-C3 GND
 *   SDA -> ESP32-C3 GPIO 8
 *   SCL -> ESP32-C3 GPIO 9
 * 
 * TTP223 Touch Sensor:
 *   VCC -> ESP32-C3 3.3V
 *   GND -> ESP32-C3 GND
 *   I/O -> ESP32-C3 GPIO 2
 * 
 * MAX98357A Audio Amplifier:
 *   Vin -> ESP32-C3 5V (or 3.3V)
 *   GND -> ESP32-C3 GND
 *   BCLK -> ESP32-C3 GPIO 4
 *   LRC -> ESP32-C3 GPIO 5
 *   DIN -> ESP32-C3 GPIO 6
 *   SD -> ESP32-C3 GPIO 7 (or tie to 3.3V)
 * 
 * Speaker:
 *   + -> MAX98357A Speaker +
 *   - -> MAX98357A Speaker -
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "driver/i2s.h"

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

bool touchState = false;
bool lastTouchState = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== Mochi Robot Component Test ===");
  
  // Initialize Display
  Serial.println("Initializing Display...");
  Wire.begin(8, 9);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("Display FAILED!");
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Display: OK");
  display.display();
  Serial.println("Display: OK");
  delay(500);
  
  // Initialize Touch
  Serial.println("Initializing Touch Sensor...");
  pinMode(TOUCH_PIN, INPUT);
  display.setCursor(0, 10);
  display.println("Touch: OK");
  display.display();
  Serial.println("Touch: OK");
  delay(500);
  
  // Initialize Audio
  Serial.println("Initializing Audio...");
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
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
  
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  pinMode(I2S_SD, OUTPUT);
  digitalWrite(I2S_SD, HIGH);
  
  display.setCursor(0, 20);
  display.println("Audio: OK");
  display.display();
  Serial.println("Audio: OK");
  delay(500);
  
  // All systems ready
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(20, 20);
  display.println("ALL OK!");
  display.setTextSize(1);
  display.setCursor(10, 45);
  display.println("Touch to test");
  display.display();
  
  Serial.println("=== All Components Ready ===");
}

void loop() {
  // Read touch sensor
  touchState = digitalRead(TOUCH_PIN);
  
  // Detect touch press (edge detection)
  if (touchState == HIGH && lastTouchState == LOW) {
    Serial.println("TOUCHED!");
    
    // Update display
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(30, 25);
    display.println("TOUCH!");
    display.display();
    
    // Play a beep
    generateTone(800, 100);
    
    delay(200);
  }
  
  lastTouchState = touchState;
  delay(10);
}

void generateTone(int frequency, int duration) {
  int sampleRate = 44100;
  int samples = (sampleRate * duration) / 1000;
  int16_t *audioBuffer = (int16_t*)malloc(samples * sizeof(int16_t));
  
  for (int i = 0; i < samples; i++) {
    audioBuffer[i] = (int16_t)(sin(2 * PI * frequency * i / sampleRate) * 16383);
  }
  
  size_t bytesWritten;
  i2s_write(I2S_NUM_0, audioBuffer, samples * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
  
  free(audioBuffer);
}

