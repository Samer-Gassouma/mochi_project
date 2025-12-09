/*
 * Mochi Robot - Touch Handler Implementation
 */

#include "touch_handler.h"
#include <Arduino.h>

TouchHandler::TouchHandler(int pin) {
  touchPin = pin;
  lastTouchState = false;
  currentTouchState = false;
  touchStartTime = 0;
  lastTapTime = 0;
  tapTimeout = 0;
  tapCount = 0;
  longPressDetected = false;
  pinMode(touchPin, INPUT);
}

void TouchHandler::update() {
  lastTouchState = currentTouchState;
  currentTouchState = digitalRead(touchPin) == HIGH;
  
  unsigned long now = millis();
  
  // Detect touch start
  if (currentTouchState && !lastTouchState) {
    touchStartTime = now;
    longPressDetected = false;
  }
  
  // Detect touch end
  if (!currentTouchState && lastTouchState) {
    unsigned long pressDuration = now - touchStartTime;
    
    // Check for long press (detected on release after 1.5s)
    if (pressDuration >= LONG_PRESS_TIME) {
      longPressDetected = true;
      tapCount = 0; // Reset tap count
      lastTapTime = 0;
    } else {
      // It's a tap
      if (now - lastTapTime < DOUBLE_TAP_WINDOW) {
        // Second tap within window = double tap
        tapCount = 2;
        lastTapTime = 0; // Reset to prevent triple tap
      } else {
        // First tap
        tapCount = 1;
        lastTapTime = now;
      }
    }
  }
  
  // Reset tap count if timeout
  if (tapCount > 0 && (now - lastTapTime) > DOUBLE_TAP_WINDOW) {
    if (tapCount == 1) {
      // Single tap confirmed
    } else {
      tapCount = 0;
    }
  }
}

TouchEvent TouchHandler::getEvent() {
  unsigned long now = millis();
  
  // Check for long press (detected during press)
  if (currentTouchState && !longPressDetected) {
    if ((now - touchStartTime) >= LONG_PRESS_TIME) {
      longPressDetected = true;
      return TOUCH_LONG_PRESS;
    }
  }
  
  // Check for double tap
  if (tapCount == 2) {
    tapCount = 0;
    return TOUCH_DOUBLE_TAP;
  }
  
  // Check for single tap (after timeout window)
  if (tapCount == 1 && (now - lastTapTime) > DOUBLE_TAP_WINDOW) {
    tapCount = 0;
    return TOUCH_SINGLE_TAP;
  }
  
  return TOUCH_NONE;
}

void TouchHandler::reset() {
  tapCount = 0;
  lastTapTime = 0;
  longPressDetected = false;
  touchStartTime = 0;
}

