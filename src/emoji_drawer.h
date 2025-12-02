/*
 * Mochi Robot - Emoji Drawing System
 * Algorithm to draw emojis on 128x64 OLED display
 */

#ifndef EMOJI_DRAWER_H
#define EMOJI_DRAWER_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Emoji types enum
enum EmojiType {
  EMOJI_HAPPY,           // 😊 Happy
  EMOJI_SAD,             // 😢 Sad/Cry
  EMOJI_ANGRY,           // 😠 Angry
  EMOJI_SURPRISED,       // 😮 Surprised
  EMOJI_LOVE,            // 😍 Love/Heart Eyes
  EMOJI_SLEEPY,          // 😴 Sleepy/Sleep
  EMOJI_THINKING,        // 🤔 Thinking
  EMOJI_LAUGHING,        // 😂 Laughing/LOL
  EMOJI_PET_HAPPY,       // 😸 Pet Happy
  EMOJI_PET_LOVE,        // 😻 Pet Love
  EMOJI_PET_ANNOYED,     // 😾 Pet Annoyed
  EMOJI_EATING,          // 😋 Eating/Nom
  EMOJI_HUNGRY,          // 🍽️ Hungry
  EMOJI_FULL,            // 😌 Full
  EMOJI_THROW_UP,        // 🤮 Throw Up
  EMOJI_STARVING,        // ☠️ Starving
  EMOJI_CRYING,          // 😢 Crying
  EMOJI_SLEEPING,        // 😴 Sleeping
  EMOJI_SICK,            // 🤒 Sick
  EMOJI_NEUTRAL          // Default neutral face
};

class EmojiDrawer {
private:
  Adafruit_SSD1306* display;
  int centerX, centerY;  // Center of emoji (typically 64, 32 for 128x64)
  int faceSize;          // Size of the face
  
  // Animation state
  unsigned long lastBlink;
  bool eyesOpen;
  int animationFrame;
  
  // Drawing helper functions
  void drawCircle(int x, int y, int radius, bool fill = false);
  void drawArc(int x, int y, int radiusX, int radiusY, int startAngle, int endAngle);
  void drawEye(int x, int y, int size, bool open = true);
  void drawMouth(int x, int y, int width, int type);
  void drawEyebrow(int x, int y, int width, bool angry = false);
  
  // Specific emoji drawing functions
  void drawHappy();
  void drawSad();
  void drawAngry();
  void drawSurprised();
  void drawLove();
  void drawSleepy();
  void drawThinking();
  void drawLaughing();
  void drawPetHappy();
  void drawPetLove();
  void drawPetAnnoyed();
  void drawEating();
  void drawHungry();
  void drawFull();
  void drawThrowUp();
  void drawStarving();
  void drawCrying();
  void drawSleeping();
  void drawSick();
  void drawNeutral();
  
public:
  EmojiDrawer(Adafruit_SSD1306* disp);
  
  // Main drawing function
  void drawEmoji(EmojiType type, int frame = 0);
  
  // Animation update (for blinking, etc.)
  void updateAnimation();
  
  // Set emoji position and size
  void setPosition(int x, int y);
  void setSize(int size);
};

#endif

