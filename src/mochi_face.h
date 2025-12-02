/*
 * Mochi Robot - State Display System
 * Shows emotion/behavior states as text on screen
 */

#ifndef MOCHI_FACE_H
#define MOCHI_FACE_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Emotion types
enum Emotion {
  EMO_IDLE,
  EMO_HAPPY,
  EMO_SAD,
  EMO_CRYING,
  EMO_ANGRY,
  EMO_SURPRISED,
  EMO_LOVE,
  EMO_SLEEPY,
  EMO_SLEEPING,
  EMO_THINKING,
  EMO_LAUGHING,
  EMO_PET_HAPPY,
  EMO_PET_LOVE,
  EMO_PET_ANNOYED,
  EMO_EATING,
  EMO_HUNGRY,
  EMO_FULL,
  EMO_THROW_UP,
  EMO_STARVING,
  EMO_SICK
};

class MochiFace {
private:
  Adafruit_SSD1306* display;
  
  // Current emotion
  Emotion currentEmotion;
  unsigned long emotionStartTime;
  int emotionDuration;
  
  // Animation state
  int animationFrame;
  
  // Get emotion name as string
  const char* getEmotionName(Emotion emotion);
  
public:
  MochiFace(Adafruit_SSD1306* disp);
  
  // Main functions
  void draw();
  void draw(int hungerPercent); // Draw with hunger percentage
  void draw(int hungerPercent, int energyPercent); // Draw with hunger and energy
  void draw(int hungerPercent, int energyPercent, bool wifiConnected); // Draw with WiFi status
  void update();
  
  // WiFi icon drawing
  void drawWiFiIcon(bool connected);
  
  // Emotion setters
  void setEmotion(Emotion emotion, int duration = 0);
  Emotion getEmotion() { return currentEmotion; }
  
  // Quick setters
  void setHappy() { setEmotion(EMO_HAPPY, 2000); }
  void setNeutral() { setEmotion(EMO_IDLE, 0); }
  void setSad() { setEmotion(EMO_SAD, 2000); }
  
  // Legacy function
  void drawIdle() { draw(); }
};

#endif
