/*
 * Mochi Robot - State Display Implementation
 * Shows emotion/behavior states as text
 */

#include "mochi_face.h"
#include <Arduino.h>
#include <math.h>

MochiFace::MochiFace(Adafruit_SSD1306* disp) {
  display = disp;
  
  // Emotion state
  currentEmotion = EMO_IDLE;
  emotionStartTime = 0;
  emotionDuration = 0;
  animationFrame = 0;
}

void MochiFace::setEmotion(Emotion emotion, int duration) {
  currentEmotion = emotion;
  emotionStartTime = millis();
  emotionDuration = duration;
  animationFrame = 0;
}

const char* MochiFace::getEmotionName(Emotion emotion) {
  switch(emotion) {
    case EMO_IDLE: return "IDLE";
    case EMO_HAPPY: return "HAPPY";
    case EMO_SAD: return "SAD";
    case EMO_CRYING: return "CRYING";
    case EMO_ANGRY: return "ANGRY";
    case EMO_SURPRISED: return "SURPRISED";
    case EMO_LOVE: return "LOVE";
    case EMO_SLEEPY: return "SLEEPY";
    case EMO_SLEEPING: return "SLEEPING";
    case EMO_THINKING: return "THINKING";
    case EMO_LAUGHING: return "LAUGHING";
    case EMO_PET_HAPPY: return "PET HAPPY";
    case EMO_PET_LOVE: return "PET LOVE";
    case EMO_PET_ANNOYED: return "PET ANNOYED";
    case EMO_EATING: return "EATING";
    case EMO_HUNGRY: return "HUNGRY";
    case EMO_FULL: return "FULL";
    case EMO_THROW_UP: return "THROWING UP";
    case EMO_STARVING: return "STARVING";
    case EMO_SICK: return "SICK";
    default: return "UNKNOWN";
  }
}

void MochiFace::update() {
  animationFrame++;
  
  // Auto-return to idle after emotion duration
  if (emotionDuration > 0 && (millis() - emotionStartTime) > emotionDuration) {
    if (currentEmotion != EMO_IDLE && currentEmotion != EMO_SLEEPING) {
      setEmotion(EMO_IDLE, 0);
    }
  }
}

void MochiFace::draw() {
  draw(-1); // Draw without hunger percentage
}

void MochiFace::draw(int hungerPercent) {
  draw(hungerPercent, -1); // Call overloaded version without energy
}

void MochiFace::draw(int hungerPercent, int energyPercent) {
  draw(hungerPercent, energyPercent, false); // Default to no WiFi
}

void MochiFace::draw(int hungerPercent, int energyPercent, bool wifiConnected) {
  display->clearDisplay();
  
  // Get emotion name
  const char* emotionName = getEmotionName(currentEmotion);
  
  // Calculate text position (centered)
  display->setTextSize(2);
  display->setTextColor(SSD1306_WHITE);
  
  // Get text width for centering (approximate)
  int textWidth = strlen(emotionName) * 12; // Approximate width per character at size 2
  int x = max(0, (128 - textWidth) / 2);
  int y = 15; // Vertical position
  
  // Draw main emotion text
  display->setCursor(x, y);
  display->print(emotionName);
  
  // Draw WiFi icon in top right
  drawWiFiIcon(wifiConnected);
  
  // Draw hunger percentage if provided
  if (hungerPercent >= 0) {
    display->setTextSize(1);
    display->setCursor(5, 5);
    display->print("H:");
    display->print(hungerPercent);
    display->print("%");
  }
  
  // Draw energy percentage if provided
  if (energyPercent >= 0) {
    display->setTextSize(1);
    display->setCursor(5, 15); // Below hunger
    display->print("E:");
    display->print(energyPercent);
    display->print("%");
  }
  
  // Add animation effects based on emotion
  animationFrame++;
  
  switch(currentEmotion) {
    case EMO_EATING:
      // Show "NOM NOM" animation
      if (animationFrame % 20 < 10) {
        display->setTextSize(1);
        display->setCursor(40, 45);
        display->print("NOM NOM");
      }
      break;
      
    case EMO_HUNGRY:
      // Show "FEED ME" message
      display->setTextSize(1);
      display->setCursor(35, 45);
      display->print("FEED ME!");
      break;
      
    case EMO_THROW_UP:
      // Show "BLEH" message
      display->setTextSize(1);
      display->setCursor(45, 45);
      display->print("BLEH!");
      break;
      
    case EMO_STARVING:
      // Show critical warning
      display->setTextSize(1);
      display->setCursor(20, 45);
      display->print("CRITICAL!");
      break;
      
    case EMO_PET_HAPPY:
      // Show "PURR" message
      if (animationFrame % 15 < 8) {
        display->setTextSize(1);
        display->setCursor(45, 45);
        display->print("PURR");
      }
      break;
      
    case EMO_PET_LOVE:
      // Show hearts
      display->setTextSize(1);
      display->setCursor(50, 45);
      display->print("<3 <3 <3");
      break;
      
    case EMO_PET_ANNOYED:
      // Show "STOP" message
      display->setTextSize(1);
      display->setCursor(45, 45);
      display->print("STOP!");
      break;
      
    case EMO_ANGRY:
      // Show "RAGE" message
      display->setTextSize(1);
      display->setCursor(45, 45);
      display->print("RAGE!");
      break;
      
    case EMO_SLEEPING: {
      // Show "Zzz" animation
      display->setTextSize(1);
      int zCount = (animationFrame / 10) % 4;
      display->setCursor(50, 45);
      for (int i = 0; i < zCount; i++) {
        display->print("Z");
      }
      break;
    }
      
    case EMO_THINKING: {
      // Show "..." animation
      display->setTextSize(1);
      int dotCount = (animationFrame / 5) % 4;
      display->setCursor(50, 45);
      for (int i = 0; i < dotCount; i++) {
        display->print(".");
      }
      break;
    }
      
    case EMO_LAUGHING:
      // Show "LOL" message
      display->setTextSize(1);
      display->setCursor(50, 45);
      display->print("LOL!");
      break;
      
    case EMO_SURPRISED:
      // Show "WOW" message
      display->setTextSize(1);
      display->setCursor(50, 45);
      display->print("WOW!");
      break;
      
    case EMO_CRYING:
      // Show "WAH" message
      display->setTextSize(1);
      display->setCursor(50, 45);
      display->print("WAH!");
      break;
      
    case EMO_SICK:
      // Show thermometer
      display->setTextSize(1);
      display->setCursor(45, 45);
      display->print("FEVER");
      break;
  }
  
  display->display();
}

void MochiFace::drawWiFiIcon(bool connected) {
  // Draw WiFi icon in top right corner (position: x=100, y=2, size: 12x12)
  int iconX = 100;
  int iconY = 2;
  int iconSize = 12;
  
  if (connected) {
    // Draw WiFi icon (connected) - 3 curved lines
    // Outer arc
    for (int i = 0; i < 180; i += 5) {
      float angle = i * PI / 180.0;
      int px = iconX + 6 + (iconSize/2 - 1) * cos(angle);
      int py = iconY + iconSize - 1 - (iconSize/2 - 1) * sin(angle);
      if (px >= 0 && px < 128 && py >= 0 && py < 64) {
        display->drawPixel(px, py, SSD1306_WHITE);
      }
    }
    // Middle arc
    for (int i = 0; i < 180; i += 5) {
      float angle = i * PI / 180.0;
      int px = iconX + 6 + (iconSize/2 - 3) * cos(angle);
      int py = iconY + iconSize - 1 - (iconSize/2 - 3) * sin(angle);
      if (px >= 0 && px < 128 && py >= 0 && py < 64) {
        display->drawPixel(px, py, SSD1306_WHITE);
      }
    }
    // Inner dot
    display->fillCircle(iconX + 6, iconY + iconSize - 1, 1, SSD1306_WHITE);
  } else {
    // Draw WiFi icon with X (not connected/AP mode)
    // Draw the WiFi arcs first
    for (int i = 0; i < 180; i += 5) {
      float angle = i * PI / 180.0;
      int px = iconX + 6 + (iconSize/2 - 1) * cos(angle);
      int py = iconY + iconSize - 1 - (iconSize/2 - 1) * sin(angle);
      if (px >= 0 && px < 128 && py >= 0 && py < 64) {
        display->drawPixel(px, py, SSD1306_WHITE);
      }
    }
    for (int i = 0; i < 180; i += 5) {
      float angle = i * PI / 180.0;
      int px = iconX + 6 + (iconSize/2 - 3) * cos(angle);
      int py = iconY + iconSize - 1 - (iconSize/2 - 3) * sin(angle);
      if (px >= 0 && px < 128 && py >= 0 && py < 64) {
        display->drawPixel(px, py, SSD1306_WHITE);
      }
    }
    display->fillCircle(iconX + 6, iconY + iconSize - 1, 1, SSD1306_WHITE);
    
    // Draw X over it (diagonal lines)
    display->drawLine(iconX + 2, iconY + 2, iconX + iconSize - 2, iconY + iconSize - 2, SSD1306_WHITE);
    display->drawLine(iconX + iconSize - 2, iconY + 2, iconX + 2, iconY + iconSize - 2, SSD1306_WHITE);
  }
}
