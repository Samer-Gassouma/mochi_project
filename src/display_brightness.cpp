/*
 * Mochi Robot - Display Brightness Control Implementation
 */

#include "display_brightness.h"
#include <Arduino.h>

DisplayBrightness::DisplayBrightness(Adafruit_SSD1306* disp) {
  display = disp;
  normalContrast = 255; // Full brightness
  dimmedContrast = 10;  // Very dim (almost off)
  currentContrast = normalContrast;
  isDimmed = false;
  isDimming = false;
  isBrightening = false;
  dimStartTime = 0;
  dimDuration = 1000;
}

void DisplayBrightness::setBrightness(uint8_t contrast) {
  currentContrast = contrast;
  // SSD1306 contrast command (0x81)
  // Use Wire directly to send contrast command
  Wire.beginTransmission(0x3C); // SSD1306 I2C address
  Wire.write(0x00); // Command mode
  Wire.write(0x81); // SETCONTRAST command
  Wire.write(contrast);
  Wire.endTransmission();
}

void DisplayBrightness::dim(unsigned long duration) {
  if (isDimmed) return; // Already dimmed
  
  dimDuration = duration;
  dimStartTime = millis();
  isDimming = true;
  isBrightening = false;
}

void DisplayBrightness::brighten(unsigned long duration) {
  if (!isDimmed) return; // Already bright
  
  dimDuration = duration;
  dimStartTime = millis();
  isBrightening = true;
  isDimming = false;
}

void DisplayBrightness::update() {
  unsigned long now = millis();
  
  if (isDimming) {
    unsigned long elapsed = now - dimStartTime;
    if (elapsed >= dimDuration) {
      // Dimming complete
      setBrightness(dimmedContrast);
      isDimmed = true;
      isDimming = false;
    } else {
      // Smooth transition
      float progress = (float)elapsed / (float)dimDuration;
      uint8_t contrast = normalContrast - (uint8_t)((normalContrast - dimmedContrast) * progress);
      setBrightness(contrast);
    }
  } else if (isBrightening) {
    unsigned long elapsed = now - dimStartTime;
    if (elapsed >= dimDuration) {
      // Brightening complete
      setBrightness(normalContrast);
      isDimmed = false;
      isBrightening = false;
    } else {
      // Smooth transition
      float progress = (float)elapsed / (float)dimDuration;
      uint8_t contrast = dimmedContrast + (uint8_t)((normalContrast - dimmedContrast) * progress);
      setBrightness(contrast);
    }
  }
}

