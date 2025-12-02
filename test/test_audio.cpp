/*
 * Mochi Robot - MAX98357A Audio Test
 * Tests the I2S audio amplifier with a simple tone
 */

#include "driver/i2s.h"

#define I2S_BCLK 4   // GPIO 4 - Bit Clock
#define I2S_LRC  5   // GPIO 5 - Word Select (Left-Right Clock)
#define I2S_DIN  6   // GPIO 6 - Data Input
#define I2S_SD   7   // GPIO 7 - Shutdown (optional)

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Initializing MAX98357A I2S Audio...");
  
  // Configure I2S
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };
  
  // Configure I2S pins
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRC,
    .data_out_num = I2S_DIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  
  // Install and start I2S driver
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  
  // Set shutdown pin high (enable amplifier)
  pinMode(I2S_SD, OUTPUT);
  digitalWrite(I2S_SD, HIGH);
  
  Serial.println("I2S Audio initialized!");
  Serial.println("Generating test tone...");
  
  // Generate a simple 440Hz tone (A4 note)
  generateTone(440, 1000); // 440Hz for 1 second
  
  Serial.println("Test complete!");
}

void loop() {
  // Generate a tone every 3 seconds
  delay(3000);
  generateTone(440, 500);
}

void generateTone(int frequency, int duration) {
  int sampleRate = 44100;
  int samples = (sampleRate * duration) / 1000;
  int16_t *audioBuffer = (int16_t*)malloc(samples * sizeof(int16_t));
  
  for (int i = 0; i < samples; i++) {
    audioBuffer[i] = (int16_t)(sin(2 * PI * frequency * i / sampleRate) * 16383);
  }
  
  size_t bytesWritten;
  i2s_write(I2S_NUM_0, audioBuffer, samples * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
  
  free(audioBuffer);
}

