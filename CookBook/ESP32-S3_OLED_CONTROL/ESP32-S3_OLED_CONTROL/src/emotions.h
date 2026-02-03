#ifndef emotions_h
#define emotions_h

#include <Adafruit_SSD1306.h>
extern Adafruit_SSD1306 display; // Declare the display object as extern 

void neutralFaceSetUp();
void neutralFaceLoop();
void talkFaceSetup();
void talkFaceLoop();
void hungerFaceLoop();
void sadFaceLoop();
void happyFaceLoop();

#endif