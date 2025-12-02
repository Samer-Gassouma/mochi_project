/*
 * Mochi Robot - SSD1306 Display Test
 * Tests the OLED display connection and basic functionality
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Initializing SSD1306 display...");
  
  // Initialize I2C with SDA=GPIO8, SCL=GPIO9
  Wire.begin(8, 9);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  Serial.println("Display initialized successfully!");
  
  // Clear the buffer
  display.clearDisplay();
  
  // Display test pattern
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println("Mochi");
  display.setCursor(20, 40);
  display.println("Robot");
  
  display.display();
  
  Serial.println("Test pattern displayed!");
}

void loop() {
  // Draw a simple animation
  static unsigned long lastUpdate = 0;
  static bool state = false;
  
  if (millis() - lastUpdate > 1000) {
    lastUpdate = millis();
    state = !state;
    
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 20);
    display.println("Mochi");
    display.setCursor(20, 40);
    display.println("Robot");
    
    // Blinking indicator
    if (state) {
      display.fillCircle(110, 10, 5, SSD1306_WHITE);
    }
    
    display.display();
    Serial.println("Display updated");
  }
}

