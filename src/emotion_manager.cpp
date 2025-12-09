/*
 * Mochi Robot - Emotion Management Implementation
 */

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <time.h>
#include "emotion_manager.h"

EmotionManager::EmotionManager(RoboEyes<Adafruit_SSD1306>* roboEyes) {
  eyes = roboEyes;
  currentEmotion = EMO_NEUTRAL;
  emotionStartTime = 0;
  emotionDuration = 0;
  emotionActive = false;
  isOnline = true;
  isInteracting = false;
  lastInteractionTime = 0;
  interactionCount = 0;
  lastRandomEmotionTime = 0;
  randomEmotionInterval = 30000; // 30 seconds default
  randomEmotionsEnabled = true;
}

void EmotionManager::update() {
  unsigned long now = millis();
  
  // Check if emotion duration has expired
  if (emotionActive && emotionDuration > 0) {
    if ((now - emotionStartTime) >= emotionDuration) {
      // Return to neutral or update based on factors
      updateEmotionFromFactors();
    }
  }
  
  // Auto-update emotion based on factors if not in active emotion
  if (!emotionActive || emotionDuration == 0) {
    updateEmotionFromFactors();
  }
  
  // Random emotion system (only when idle and online)
  if (randomEmotionsEnabled && isOnline && !isInteracting && 
      (now - lastInteractionTime) > 10000 && // At least 10 seconds since last interaction
      (now - lastRandomEmotionTime) > randomEmotionInterval) {
    // Random interval between 20-60 seconds
    randomEmotionInterval = 20000 + (random(40000));
    lastRandomEmotionTime = now;
    
    // 30% chance to trigger random emotion when idle
    if (random(100) < 30) {
      triggerRandomEmotion();
    }
  }
}

void EmotionManager::setEmotion(MochiEmotion emotion, unsigned long duration) {
  currentEmotion = emotion;
  emotionStartTime = millis();
  emotionDuration = duration;
  emotionActive = (duration > 0);
  
  applyEmotionToRoboEyes(emotion);
}

void EmotionManager::setInteracting(bool interacting) {
  isInteracting = interacting;
  if (interacting) {
    lastInteractionTime = millis();
    interactionCount++;
  }
}

void EmotionManager::updateEmotionFromFactors() {
  // Don't override active emotions unless they're done
  if (emotionActive && emotionDuration > 0) {
    return;
  }
  
  unsigned long now = millis();
  
  // Priority 1: Offline = worried
  if (!isOnline) {
    if (currentEmotion != EMO_WORRIED) {
      setEmotion(EMO_WORRIED, 0);
    }
    return;
  }
  
  // Priority 2: Recent interaction = excited
  if (isInteracting && (now - lastInteractionTime) < 2000) {
    if (interactionCount >= 2) {
      if (currentEmotion != EMO_EXCITED) {
        setEmotion(EMO_EXCITED, 1500);
      }
      return;
    }
  }
  
  // Priority 3: Time-based emotions (morning = happy, night = sleepy)
  struct tm timeInfo;
  if (getLocalTime(&timeInfo)) {
    int hour = timeInfo.tm_hour;
    
    // Night time (22:00 - 06:00) = sleepy
    if (hour >= 22 || hour < 6) {
      if (currentEmotion != EMO_SLEEPY && (now - lastInteractionTime) > 30000) {
        setEmotion(EMO_SLEEPY, 0);
        return;
      }
    }
    // Morning (06:00 - 10:00) = happy
    else if (hour >= 6 && hour < 10) {
      if (currentEmotion != EMO_HAPPY && (now - lastInteractionTime) > 60000) {
        // 20% chance to be happy in the morning
        if (random(100) < 20) {
          setEmotion(EMO_HAPPY, 3000);
          return;
        }
      }
    }
  }
  
  // Priority 4: Inactivity = idle/bored
  if ((now - lastInteractionTime) > 60000) { // 1 minute
    if (currentEmotion != EMO_IDLE) {
      setEmotion(EMO_IDLE, 0);
    }
    return;
  }
  
  // Default: neutral
  if (currentEmotion != EMO_NEUTRAL) {
    setEmotion(EMO_NEUTRAL, 0);
  }
}

void EmotionManager::applyEmotionToRoboEyes(MochiEmotion emotion) {
  switch(emotion) {
    case EMO_NEUTRAL:
      eyes->setMood(DEFAULT);
      eyes->setIdleMode(ON, 3, 2); // Idle mode: look around every 3-5 seconds
      eyes->setAutoblinker(ON, 3, 2); // Auto blink every 3-5 seconds
      break;
      
    case EMO_HAPPY:
      eyes->setMood(HAPPY);
      eyes->setIdleMode(OFF);
      eyes->setAutoblinker(ON, 2, 1);
      break;
      
    case EMO_SLEEPY:
      eyes->setMood(TIRED);
      eyes->setIdleMode(OFF);
      eyes->setAutoblinker(OFF);
      break;
      
    case EMO_SAD:
      eyes->setMood(DEFAULT);
      eyes->setPosition(S); // Look down
      eyes->setIdleMode(OFF);
      eyes->setAutoblinker(ON, 4, 2);
      break;
      
    case EMO_ANGRY:
      eyes->setMood(ANGRY);
      eyes->setIdleMode(OFF);
      eyes->setAutoblinker(OFF);
      break;
      
    case EMO_EXCITED:
      eyes->setMood(HAPPY);
      eyes->anim_laugh(); // Play laugh animation
      eyes->setIdleMode(OFF);
      eyes->setAutoblinker(ON, 1, 1);
      break;
      
    case EMO_IDLE:
      eyes->setMood(DEFAULT);
      eyes->setIdleMode(ON, 5, 3); // Look around every 5-8 seconds
      eyes->setAutoblinker(ON, 4, 2);
      break;
      
    case EMO_WORRIED:
      eyes->setMood(DEFAULT);
      eyes->setSweat(ON); // Show sweat
      eyes->setIdleMode(OFF);
      eyes->setAutoblinker(ON, 2, 1);
      break;
  }
}

MochiEmotion EmotionManager::getRandomEmotion() {
  // Exclude worried (only for offline) and current emotion
  MochiEmotion emotions[] = {EMO_HAPPY, EMO_SLEEPY, EMO_SAD, EMO_ANGRY, EMO_EXCITED, EMO_IDLE, EMO_NEUTRAL};
  int count = sizeof(emotions) / sizeof(emotions[0]);
  
  MochiEmotion randomEmo;
  do {
    randomEmo = emotions[random(count)];
  } while (randomEmo == currentEmotion && count > 1); // Avoid same emotion
  
  return randomEmo;
}

void EmotionManager::triggerRandomEmotion() {
  MochiEmotion randomEmo = getRandomEmotion();
  
  // Set duration based on emotion type
  unsigned long duration = 0;
  switch(randomEmo) {
    case EMO_HAPPY:
    case EMO_EXCITED:
      duration = 2000 + random(1000); // 2-3 seconds
      break;
    case EMO_SAD:
    case EMO_ANGRY:
      duration = 1500 + random(500); // 1.5-2 seconds
      break;
    case EMO_SLEEPY:
    case EMO_IDLE:
    case EMO_NEUTRAL:
      duration = 0; // Persistent
      break;
    default:
      duration = 0;
  }
  
  setEmotion(randomEmo, duration);
  
  // Play random animation based on emotion
  switch(randomEmo) {
    case EMO_HAPPY:
      // 50% chance to laugh
      if (random(100) < 50) {
        eyes->anim_laugh();
      }
      break;
    case EMO_EXCITED:
      eyes->anim_laugh();
      break;
    case EMO_SAD:
      // Sometimes confused, sometimes just look down
      if (random(100) < 30) {
        eyes->anim_confused();
      }
      break;
    case EMO_NEUTRAL:
      // Occasionally do a quick animation
      if (random(100) < 20) {
        eyes->anim_confused();
      }
      break;
    default:
      break;
  }
  
  Serial.print("🎲 Random emotion triggered: ");
  switch(randomEmo) {
    case EMO_HAPPY: Serial.println("Happy"); break;
    case EMO_SLEEPY: Serial.println("Sleepy"); break;
    case EMO_SAD: Serial.println("Sad"); break;
    case EMO_ANGRY: Serial.println("Angry"); break;
    case EMO_EXCITED: Serial.println("Excited"); break;
    case EMO_IDLE: Serial.println("Idle"); break;
    case EMO_NEUTRAL: Serial.println("Neutral"); break;
    default: Serial.println("Unknown"); break;
  }
}

