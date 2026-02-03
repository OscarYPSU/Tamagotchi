#ifndef display_h
#define display_h
#include <Adafruit_SSD1306.h>

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Declaration for SSD1306 display
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
extern Adafruit_SSD1306 display; // Declare the display object as extern


#endif