/*
 * Mochi Robot - TTP223 Touch Sensor Test
 * Tests the touch sensor and prints status to serial
 */

#define TOUCH_PIN 2  // GPIO 2 connected to TTP223 I/O pin

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(TOUCH_PIN, INPUT);
  
  Serial.println("TTP223 Touch Sensor Test");
  Serial.println("Touch the sensor to see the output...");
}

void loop() {
  int touchValue = digitalRead(TOUCH_PIN);
  
  if (touchValue == HIGH) {
    Serial.println("TOUCHED!");
  }
  
  delay(100); // Small delay to avoid spam
}

