/*
 * Mochi Robot - Emoji System Test
 * Tests all emoji drawings on the display
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "emoji_drawer.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
EmojiDrawer emojiDrawer(&display);

EmojiType emojis[] = {
  EMOJI_HAPPY,
  EMOJI_SAD,
  EMOJI_ANGRY,
  EMOJI_SURPRISED,
  EMOJI_LOVE,
  EMOJI_SLEEPY,
  EMOJI_THINKING,
  EMOJI_LAUGHING,
  EMOJI_PET_HAPPY,
  EMOJI_PET_LOVE,
  EMOJI_PET_ANNOYED,
  EMOJI_EATING,
  EMOJI_HUNGRY,
  EMOJI_FULL,
  EMOJI_THROW_UP,
  EMOJI_STARVING,
  EMOJI_CRYING,
  EMOJI_SLEEPING,
  EMOJI_SICK,
  EMOJI_NEUTRAL
};

int currentEmoji = 0;
unsigned long lastChange = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Initializing Emoji Test...");
  
  // Initialize display
  Wire.begin(8, 9);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  emojiDrawer.setPosition(64, 32);
  emojiDrawer.setSize(40);
  
  Serial.println("Display ready! Cycling through emojis...");
}

void loop() {
  // Cycle through emojis every 2 seconds
  if (millis() - lastChange > 2000) {
    lastChange = millis();
    
    emojiDrawer.updateAnimation();
    emojiDrawer.drawEmoji(emojis[currentEmoji], millis() / 50);
    
    Serial.print("Showing emoji: ");
    Serial.println(currentEmoji);
    
    currentEmoji = (currentEmoji + 1) % (sizeof(emojis) / sizeof(emojis[0]));
  } else {
    // Update animation while waiting
    emojiDrawer.updateAnimation();
    emojiDrawer.drawEmoji(emojis[currentEmoji], millis() / 50);
  }
  
  delay(50);
}

