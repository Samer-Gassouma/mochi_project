/*
 * Mochi Robot - Emoji Drawing Implementation
 * Pixel-based algorithm to draw emojis on 128x64 OLED
 */

#include "emoji_drawer.h"
#include <math.h>

EmojiDrawer::EmojiDrawer(Adafruit_SSD1306* disp) {
  display = disp;
  centerX = 64;
  centerY = 32;
  faceSize = 40;
  lastBlink = 0;
  eyesOpen = true;
  animationFrame = 0;
}

void EmojiDrawer::setPosition(int x, int y) {
  centerX = x;
  centerY = y;
}

void EmojiDrawer::setSize(int size) {
  faceSize = size;
}

void EmojiDrawer::drawCircle(int x, int y, int radius, bool fill) {
  if (fill) {
    display->fillCircle(x, y, radius, SSD1306_WHITE);
  } else {
    display->drawCircle(x, y, radius, SSD1306_WHITE);
  }
}

void EmojiDrawer::drawArc(int x, int y, int radiusX, int radiusY, int startAngle, int endAngle) {
  // Draw an arc by plotting points along the ellipse
  // Angles are in degrees, 0 is right, 90 is down
  int steps = abs(endAngle - startAngle) / 2;
  if (steps < 1) steps = 1;
  
  for (int i = 0; i <= steps; i++) {
    int angle = startAngle + (endAngle - startAngle) * i / steps;
    float rad = angle * PI / 180.0;
    int px = x + radiusX * cos(rad);
    int py = y + radiusY * sin(rad);
    display->drawPixel(px, py, SSD1306_WHITE);
    
    // Draw a few pixels around for thickness
    if (i > 0 && i < steps) {
      display->drawPixel(px + 1, py, SSD1306_WHITE);
      display->drawPixel(px, py + 1, SSD1306_WHITE);
    }
  }
}

void EmojiDrawer::drawEye(int x, int y, int size, bool open) {
  if (open) {
    // Open eye - draw circle
    display->fillCircle(x, y, size, SSD1306_WHITE);
    // Pupil
    display->fillCircle(x, y, size/3, SSD1306_BLACK);
  } else {
    // Closed eye - draw line
    display->drawLine(x - size, y, x + size, y, SSD1306_WHITE);
  }
}

void EmojiDrawer::drawMouth(int x, int y, int width, int type) {
  // type: 0=happy (smile), 1=sad (frown), 2=surprised (O), 3=neutral (line)
  switch(type) {
    case 0: // Happy smile
      drawArc(x, y, width/2, width/3, 0, 180);
      break;
    case 1: // Sad frown
      drawArc(x, y, width/2, width/3, 180, 360);
      break;
    case 2: // Surprised O
      display->drawCircle(x, y, width/2, SSD1306_WHITE);
      break;
    case 3: // Neutral line
      display->drawLine(x - width/2, y, x + width/2, y, SSD1306_WHITE);
      break;
  }
}

void EmojiDrawer::drawEyebrow(int x, int y, int width, bool angry) {
  if (angry) {
    // Angry eyebrows (V shape)
    display->drawLine(x - width/2, y, x, y - 3, SSD1306_WHITE);
    display->drawLine(x, y - 3, x + width/2, y, SSD1306_WHITE);
  } else {
    // Normal eyebrows
    display->drawLine(x - width/2, y, x + width/2, y, SSD1306_WHITE);
  }
}

void EmojiDrawer::updateAnimation() {
  // Handle blinking animation
  unsigned long now = millis();
  if (now - lastBlink > 3000) { // Blink every 3 seconds
    eyesOpen = !eyesOpen;
    lastBlink = now;
  }
  animationFrame++;
}

// ========== EMOJI DRAWING FUNCTIONS ==========

void EmojiDrawer::drawHappy() {
  int eyeY = centerY - 8;
  int eyeSpacing = 12;
  
  // Face circle
  drawCircle(centerX, centerY, faceSize/2, false);
  
  // Eyes (with blink)
  drawEye(centerX - eyeSpacing, eyeY, 4, eyesOpen);
  drawEye(centerX + eyeSpacing, eyeY, 4, eyesOpen);
  
  // Happy smile
  drawArc(centerX, centerY + 8, 12, 6, 0, 180);
}

void EmojiDrawer::drawSad() {
  int eyeY = centerY - 8;
  int eyeSpacing = 12;
  
  // Face circle
  drawCircle(centerX, centerY, faceSize/2, false);
  
  // Sad eyes
  drawEye(centerX - eyeSpacing, eyeY, 4, true);
  drawEye(centerX + eyeSpacing, eyeY, 4, true);
  
  // Tears
  display->fillCircle(centerX - eyeSpacing, eyeY + 6, 2, SSD1306_WHITE);
  display->fillCircle(centerX + eyeSpacing, eyeY + 6, 2, SSD1306_WHITE);
  
  // Sad frown
  drawArc(centerX, centerY + 12, 10, 5, 180, 360);
}

void EmojiDrawer::drawAngry() {
  int eyeY = centerY - 8;
  int eyeSpacing = 12;
  
  // Face circle
  drawCircle(centerX, centerY, faceSize/2, false);
  
  // Angry eyebrows
  drawEyebrow(centerX - eyeSpacing, eyeY - 4, 8, true);
  drawEyebrow(centerX + eyeSpacing, eyeY - 4, 8, true);
  
  // Angry eyes (smaller)
  drawEye(centerX - eyeSpacing, eyeY, 3, true);
  drawEye(centerX + eyeSpacing, eyeY, 3, true);
  
  // Angry mouth
  display->drawLine(centerX - 8, centerY + 10, centerX + 8, centerY + 10, SSD1306_WHITE);
  
  // Steam (optional animation)
  if (animationFrame % 20 < 10) {
    display->fillCircle(centerX - 10, centerY - faceSize/2 - 2, 2, SSD1306_WHITE);
    display->fillCircle(centerX + 10, centerY - faceSize/2 - 2, 2, SSD1306_WHITE);
  }
}

void EmojiDrawer::drawSurprised() {
  int eyeY = centerY - 8;
  int eyeSpacing = 12;
  
  // Face circle
  drawCircle(centerX, centerY, faceSize/2, false);
  
  // Wide open eyes
  drawCircle(centerX - eyeSpacing, eyeY, 6, false);
  drawCircle(centerX + eyeSpacing, eyeY, 6, false);
  display->fillCircle(centerX - eyeSpacing, eyeY, 3, SSD1306_WHITE);
  display->fillCircle(centerX + eyeSpacing, eyeY, 3, SSD1306_WHITE);
  
  // Surprised O mouth
  drawCircle(centerX, centerY + 10, 6, false);
}

void EmojiDrawer::drawLove() {
  int eyeY = centerY - 8;
  int eyeSpacing = 12;
  
  // Face circle
  drawCircle(centerX, centerY, faceSize/2, false);
  
  // Heart eyes
  // Left heart
  display->fillCircle(centerX - eyeSpacing - 2, eyeY, 3, SSD1306_WHITE);
  display->fillCircle(centerX - eyeSpacing + 2, eyeY, 3, SSD1306_WHITE);
  display->fillTriangle(centerX - eyeSpacing, eyeY + 4, centerX - eyeSpacing - 4, eyeY, centerX - eyeSpacing + 4, eyeY, SSD1306_WHITE);
  
  // Right heart
  display->fillCircle(centerX + eyeSpacing - 2, eyeY, 3, SSD1306_WHITE);
  display->fillCircle(centerX + eyeSpacing + 2, eyeY, 3, SSD1306_WHITE);
  display->fillTriangle(centerX + eyeSpacing, eyeY + 4, centerX + eyeSpacing - 4, eyeY, centerX + eyeSpacing + 4, eyeY, SSD1306_WHITE);
  
  // Happy smile
  drawArc(centerX, centerY + 8, 12, 6, 0, 180);
  
  // Blush (optional)
  display->fillCircle(centerX - 18, centerY + 2, 3, SSD1306_WHITE);
  display->fillCircle(centerX + 18, centerY + 2, 3, SSD1306_WHITE);
}

void EmojiDrawer::drawSleepy() {
  int eyeY = centerY - 8;
  int eyeSpacing = 12;
  
  // Face circle
  drawCircle(centerX, centerY, faceSize/2, false);
  
  // Droopy eyes
  drawArc(centerX - eyeSpacing, eyeY, 4, 2, 0, 180);
  drawArc(centerX + eyeSpacing, eyeY, 4, 2, 0, 180);
  
  // Z's floating up (animation)
  int zOffset = (animationFrame % 30) - 15;
  display->drawLine(centerX - 5, centerY - faceSize/2 + zOffset, centerX - 2, centerY - faceSize/2 - 2 + zOffset, SSD1306_WHITE);
  display->drawLine(centerX - 2, centerY - faceSize/2 - 2 + zOffset, centerX + 1, centerY - faceSize/2 + zOffset, SSD1306_WHITE);
  display->drawLine(centerX + 1, centerY - faceSize/2 + zOffset, centerX + 4, centerY - faceSize/2 - 2 + zOffset, SSD1306_WHITE);
}

void EmojiDrawer::drawThinking() {
  int eyeY = centerY - 8;
  int eyeSpacing = 12;
  
  // Face circle
  drawCircle(centerX, centerY, faceSize/2, false);
  
  // Eyes looking up/thinking
  display->fillCircle(centerX - eyeSpacing, eyeY - 2, 3, SSD1306_WHITE);
  display->fillCircle(centerX + eyeSpacing, eyeY - 2, 3, SSD1306_WHITE);
  
  // Hand on chin (simplified)
  display->drawLine(centerX, centerY + 5, centerX, centerY + 15, SSD1306_WHITE);
  display->fillCircle(centerX, centerY + 15, 4, SSD1306_WHITE);
  
  // Thought bubble
  display->drawCircle(centerX + 15, centerY - 10, 5, SSD1306_WHITE);
  display->drawCircle(centerX + 20, centerY - 15, 3, SSD1306_WHITE);
}

void EmojiDrawer::drawLaughing() {
  int eyeY = centerY - 8;
  int eyeSpacing = 12;
  
  // Face circle
  drawCircle(centerX, centerY, faceSize/2, false);
  
  // Squinted eyes (laughing)
  drawArc(centerX - eyeSpacing, eyeY, 5, 2, 0, 180);
  drawArc(centerX + eyeSpacing, eyeY, 5, 2, 0, 180);
  
  // Tears of joy
  display->fillCircle(centerX - eyeSpacing, eyeY + 4, 2, SSD1306_WHITE);
  display->fillCircle(centerX + eyeSpacing, eyeY + 4, 2, SSD1306_WHITE);
  
  // Wide open laughing mouth
  display->fillRect(centerX - 10, centerY + 8, 20, 8, SSD1306_WHITE);
  // Teeth
  for (int i = -8; i <= 8; i += 4) {
    display->drawLine(centerX + i, centerY + 8, centerX + i, centerY + 12, SSD1306_BLACK);
  }
}

void EmojiDrawer::drawPetHappy() {
  drawHappy();
  // Add some extra sparkle
  if (animationFrame % 10 < 5) {
    display->fillCircle(centerX - 20, centerY - 15, 1, SSD1306_WHITE);
    display->fillCircle(centerX + 20, centerY - 15, 1, SSD1306_WHITE);
  }
}

void EmojiDrawer::drawPetLove() {
  drawLove();
  // Extra hearts around
  int heartFrame = animationFrame % 20;
  if (heartFrame < 10) {
    display->fillCircle(centerX - 25, centerY - 20, 2, SSD1306_WHITE);
    display->fillCircle(centerX + 25, centerY - 20, 2, SSD1306_WHITE);
  }
}

void EmojiDrawer::drawPetAnnoyed() {
  int eyeY = centerY - 8;
  int eyeSpacing = 12;
  
  // Face circle
  drawCircle(centerX, centerY, faceSize/2, false);
  
  // Annoyed eyes (looking away)
  display->fillCircle(centerX - eyeSpacing - 2, eyeY, 3, SSD1306_WHITE);
  display->fillCircle(centerX + eyeSpacing + 2, eyeY, 3, SSD1306_WHITE);
  
  // Slight frown
  drawArc(centerX, centerY + 10, 8, 3, 180, 360);
}

void EmojiDrawer::drawEating() {
  int eyeY = centerY - 8;
  int eyeSpacing = 12;
  
  // Face circle
  drawCircle(centerX, centerY, faceSize/2, false);
  
  // Happy eyes
  drawEye(centerX - eyeSpacing, eyeY, 4, eyesOpen);
  drawEye(centerX + eyeSpacing, eyeY, 4, eyesOpen);
  
  // Eating mouth (chewing animation)
  int mouthFrame = animationFrame % 12;
  if (mouthFrame < 6) {
    display->fillRect(centerX - 8, centerY + 8, 16, 6, SSD1306_WHITE);
  } else {
    display->fillRect(centerX - 6, centerY + 8, 12, 6, SSD1306_WHITE);
  }
}

void EmojiDrawer::drawHungry() {
  int eyeY = centerY - 8;
  int eyeSpacing = 12;
  
  // Face circle
  drawCircle(centerX, centerY, faceSize/2, false);
  
  // Big eyes looking at food
  drawEye(centerX - eyeSpacing, eyeY, 5, true);
  drawEye(centerX + eyeSpacing, eyeY, 5, true);
  
  // Open mouth (wanting food)
  display->drawCircle(centerX, centerY + 10, 5, SSD1306_WHITE);
  
  // Fork/spoon icon (simplified)
  display->drawLine(centerX + 20, centerY - 5, centerX + 20, centerY + 5, SSD1306_WHITE);
  display->drawLine(centerX + 18, centerY - 5, centerX + 22, centerY - 5, SSD1306_WHITE);
}

void EmojiDrawer::drawFull() {
  int eyeY = centerY - 8;
  int eyeSpacing = 12;
  
  // Face circle
  drawCircle(centerX, centerY, faceSize/2, false);
  
  // Content eyes
  drawArc(centerX - eyeSpacing, eyeY, 4, 2, 0, 180);
  drawArc(centerX + eyeSpacing, eyeY, 4, 2, 0, 180);
  
  // Small satisfied smile
  drawArc(centerX, centerY + 8, 8, 3, 0, 180);
}

void EmojiDrawer::drawThrowUp() {
  int eyeY = centerY - 8;
  int eyeSpacing = 12;
  
  // Face circle
  drawCircle(centerX, centerY, faceSize/2, false);
  
  // Unhappy eyes
  drawEye(centerX - eyeSpacing, eyeY, 4, true);
  drawEye(centerX + eyeSpacing, eyeY, 4, true);
  
  // Open mouth with "vomit" (green would be ideal but we use white)
  display->fillRect(centerX - 6, centerY + 8, 12, 10, SSD1306_WHITE);
  // Vomit particles
  for (int i = 0; i < 5; i++) {
    int offset = (animationFrame + i * 3) % 15;
    display->fillCircle(centerX - 10 + i * 5, centerY + 18 + offset, 2, SSD1306_WHITE);
  }
}

void EmojiDrawer::drawStarving() {
  int eyeY = centerY - 8;
  int eyeSpacing = 12;
  
  // Face circle (thinner)
  drawCircle(centerX, centerY, faceSize/2 - 2, false);
  
  // Weak eyes
  display->drawLine(centerX - eyeSpacing - 3, eyeY, centerX - eyeSpacing + 3, eyeY, SSD1306_WHITE);
  display->drawLine(centerX + eyeSpacing - 3, eyeY, centerX + eyeSpacing + 3, eyeY, SSD1306_WHITE);
  
  // Weak mouth
  display->drawLine(centerX - 4, centerY + 10, centerX + 4, centerY + 10, SSD1306_WHITE);
  
  // Skull crossbones (simplified)
  display->drawLine(centerX - 15, centerY - 20, centerX + 15, centerY - 20, SSD1306_WHITE);
  display->drawLine(centerX, centerY - 25, centerX, centerY - 15, SSD1306_WHITE);
}

void EmojiDrawer::drawCrying() {
  drawSad();
  // Extra tears
  for (int i = 0; i < 3; i++) {
    int tearY = centerY - 5 + (animationFrame + i * 5) % 20;
    display->fillCircle(centerX - 12, tearY, 1, SSD1306_WHITE);
    display->fillCircle(centerX + 12, tearY, 1, SSD1306_WHITE);
  }
}

void EmojiDrawer::drawSleeping() {
  int eyeY = centerY - 8;
  int eyeSpacing = 12;
  
  // Face circle
  drawCircle(centerX, centerY, faceSize/2, false);
  
  // Closed eyes (sleeping)
  drawArc(centerX - eyeSpacing, eyeY, 5, 2, 0, 180);
  drawArc(centerX + eyeSpacing, eyeY, 5, 2, 0, 180);
  
  // Z's floating
  int zOffset = (animationFrame % 40) - 20;
  for (int i = 0; i < 3; i++) {
    int zX = centerX - 10 + i * 10;
    int zY = centerY - faceSize/2 + zOffset + i * 5;
    display->drawLine(zX - 2, zY, zX + 2, zY - 2, SSD1306_WHITE);
    display->drawLine(zX + 2, zY - 2, zX - 2, zY - 4, SSD1306_WHITE);
    display->drawLine(zX - 2, zY - 4, zX + 2, zY - 6, SSD1306_WHITE);
  }
}

void EmojiDrawer::drawSick() {
  int eyeY = centerY - 8;
  int eyeSpacing = 12;
  
  // Face circle
  drawCircle(centerX, centerY, faceSize/2, false);
  
  // Tired eyes
  drawEye(centerX - eyeSpacing, eyeY, 4, true);
  drawEye(centerX + eyeSpacing, eyeY, 4, true);
  
  // Thermometer (simplified)
  display->drawLine(centerX + 18, centerY - 15, centerX + 18, centerY - 5, SSD1306_WHITE);
  display->fillRect(centerX + 17, centerY - 15, 3, 5, SSD1306_WHITE);
  
  // Weak mouth
  display->drawLine(centerX - 6, centerY + 10, centerX + 6, centerY + 10, SSD1306_WHITE);
}

void EmojiDrawer::drawNeutral() {
  int eyeY = centerY - 8;
  int eyeSpacing = 12;
  
  // Face circle
  drawCircle(centerX, centerY, faceSize/2, false);
  
  // Neutral eyes
  drawEye(centerX - eyeSpacing, eyeY, 4, eyesOpen);
  drawEye(centerX + eyeSpacing, eyeY, 4, eyesOpen);
  
  // Neutral mouth
  display->drawLine(centerX - 8, centerY + 10, centerX + 8, centerY + 10, SSD1306_WHITE);
}

// Main drawing function
void EmojiDrawer::drawEmoji(EmojiType type, int frame) {
  animationFrame = frame;
  display->clearDisplay();
  
  switch(type) {
    case EMOJI_HAPPY: drawHappy(); break;
    case EMOJI_SAD: drawSad(); break;
    case EMOJI_ANGRY: drawAngry(); break;
    case EMOJI_SURPRISED: drawSurprised(); break;
    case EMOJI_LOVE: drawLove(); break;
    case EMOJI_SLEEPY: drawSleepy(); break;
    case EMOJI_THINKING: drawThinking(); break;
    case EMOJI_LAUGHING: drawLaughing(); break;
    case EMOJI_PET_HAPPY: drawPetHappy(); break;
    case EMOJI_PET_LOVE: drawPetLove(); break;
    case EMOJI_PET_ANNOYED: drawPetAnnoyed(); break;
    case EMOJI_EATING: drawEating(); break;
    case EMOJI_HUNGRY: drawHungry(); break;
    case EMOJI_FULL: drawFull(); break;
    case EMOJI_THROW_UP: drawThrowUp(); break;
    case EMOJI_STARVING: drawStarving(); break;
    case EMOJI_CRYING: drawCrying(); break;
    case EMOJI_SLEEPING: drawSleeping(); break;
    case EMOJI_SICK: drawSick(); break;
    case EMOJI_NEUTRAL: drawNeutral(); break;
  }
  
  display->display();
}

