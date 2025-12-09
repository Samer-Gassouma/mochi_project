/*
 * Mochi Robot - Display Brightness Control
 * Handles OLED dimming for sleep mode
 */

#ifndef DISPLAY_BRIGHTNESS_H
#define DISPLAY_BRIGHTNESS_H

#include <Adafruit_SSD1306.h>

class DisplayBrightness {
private:
  Adafruit_SSD1306* display;
  uint8_t normalContrast;
  uint8_t dimmedContrast;
  uint8_t currentContrast;
  bool isDimmed;
  
  // Dimming animation
  unsigned long dimStartTime;
  unsigned long dimDuration;
  bool isDimming;
  bool isBrightening;
  
public:
  DisplayBrightness(Adafruit_SSD1306* disp);
  
  // Set contrast levels
  void setNormalContrast(uint8_t contrast) { normalContrast = contrast; }
  void setDimmedContrast(uint8_t contrast) { dimmedContrast = contrast; }
  
  // Dimming control
  void dim(unsigned long duration = 1000);
  void brighten(unsigned long duration = 1000);
  void setBrightness(uint8_t contrast);
  
  // Update (call in loop for smooth transitions)
  void update();
  
  // Check state
  bool getIsDimmed() { return isDimmed; }
};

#endif

