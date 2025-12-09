/*
 * Mochi Robot - Touch Handler
 * Detects single tap, double tap, and long press
 */

#ifndef TOUCH_HANDLER_H
#define TOUCH_HANDLER_H

// Touch event types
enum TouchEvent {
  TOUCH_NONE = 0,
  TOUCH_SINGLE_TAP,
  TOUCH_DOUBLE_TAP,
  TOUCH_LONG_PRESS
};

class TouchHandler {
private:
  int touchPin;
  bool lastTouchState;
  bool currentTouchState;
  unsigned long touchStartTime;
  unsigned long lastTapTime;
  unsigned long tapTimeout;
  int tapCount;
  bool longPressDetected;
  
  // Timing constants
  static const unsigned long LONG_PRESS_TIME = 1500;  // 1.5 seconds
  static const unsigned long DOUBLE_TAP_WINDOW = 400; // 400ms window for double tap
  
public:
  TouchHandler(int pin);
  
  // Update touch state (call in loop)
  void update();
  
  // Get current touch event
  TouchEvent getEvent();
  
  // Check if currently touching
  bool isTouching() { return currentTouchState; }
  
  // Reset touch state
  void reset();
};

#endif

