#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <display.h>
#include <emotions.h>
#include <milis.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // Standard I2C address

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);

  // For ESP32-S3: Initialize I2C on pins 8 (SDA) and 9 (SCL)
  Wire.begin(8, 9);

  // initialize with the I2C addr 0x3C
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed. Check wiring or I2C address."));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(2);      // Bigger text for the test
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println(F("POWER ON"));
  display.display(); 
  
  Serial.println(F("OLED Initialized Successfully!"));

  talkFaceSetup();
  Serial.println("Setting up talking face");
}

void loop() {
  // Just sit here and stay on
  talkFaceLoop();
}