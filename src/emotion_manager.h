/*
 * Mochi Robot - Emotion Management System
 * Maps emotions to RoboEyes library moods and animations
 */

#ifndef EMOTION_MANAGER_H
#define EMOTION_MANAGER_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "../RoboEyes/src/FluxGarage_RoboEyes.h"

// Emotion types (mapped to RoboEyes)
enum MochiEmotion {
  EMO_NEUTRAL = 0,
  EMO_HAPPY,
  EMO_SLEEPY,
  EMO_SAD,
  EMO_ANGRY,
  EMO_EXCITED,
  EMO_IDLE,
  EMO_WORRIED  // For offline/error states
};

class EmotionManager {
private:
  RoboEyes<Adafruit_SSD1306>* eyes;
  MochiEmotion currentEmotion;
  unsigned long emotionStartTime;
  unsigned long emotionDuration;
  bool emotionActive;
  
  // Emotion factors
  bool isOnline;
  bool isInteracting;
  unsigned long lastInteractionTime;
  int interactionCount;
  
  // Random emotion system
  unsigned long lastRandomEmotionTime;
  unsigned long randomEmotionInterval;
  bool randomEmotionsEnabled;
  
public:
  EmotionManager(RoboEyes<Adafruit_SSD1306>* roboEyes);
  
  // Main functions
  void update();
  void setEmotion(MochiEmotion emotion, unsigned long duration = 0);
  MochiEmotion getCurrentEmotion() { return currentEmotion; }
  
  // Emotion factors
  void setOnline(bool online) { isOnline = online; updateEmotionFromFactors(); }
  void setInteracting(bool interacting);
  void setInteractionCount(int count) { interactionCount = count; }
  
  // Quick setters
  void setHappy() { setEmotion(EMO_HAPPY, 2000); }
  void setNeutral() { setEmotion(EMO_NEUTRAL, 0); }
  void setSleepy() { setEmotion(EMO_SLEEPY, 0); }
  void setSad() { setEmotion(EMO_SAD, 2000); }
  void setAngry() { setEmotion(EMO_ANGRY, 2000); }
  void setExcited() { setEmotion(EMO_EXCITED, 1500); }
  void setWorried() { setEmotion(EMO_WORRIED, 0); }
  
  // Random emotion system
  void enableRandomEmotions(bool enable) { randomEmotionsEnabled = enable; }
  void triggerRandomEmotion(); // Trigger a random emotion
  MochiEmotion getRandomEmotion(); // Get a random emotion type
  
private:
  void updateEmotionFromFactors();
  void applyEmotionToRoboEyes(MochiEmotion emotion);
};

#endif

