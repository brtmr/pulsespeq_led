#include <LiquidCrystal.h>
#include <Adafruit_NeoPixel.h>

#define PIN 6
#define PIXELS 36

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXELS, PIN, NEO_GRB + NEO_KHZ800);

uint8_t signal;
float max_;
uint32_t counter = 0;
boolean found;
const uint32_t GREEN = Adafruit_NeoPixel::Color(0,255,0);
const uint32_t YELLOW = Adafruit_NeoPixel::Color(255,255,0);
const uint32_t RED = Adafruit_NeoPixel::Color(255,0,0);
const uint32_t WHITE = Adafruit_NeoPixel::Color(255,255,255);
uint32_t black = 0;

/* creates a character set of 8 bars of different height */
void setup(){
  Serial.begin(9600);
  signal = 15;
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  //color = Wheel(128);
}

void loop(){
  counter++;
  while ( Serial.available() > 0 ) {
    signal = Serial.read();
  } 
  signal = signal % 36;
  draw_signal();
}

void draw_signal(){
  for (int i=0; i<signal; i++){
    strip.setPixelColor(i, getColor(i));
  }
  for (int i=signal; i<PIXELS; i++){
    strip.setPixelColor(i, black);
  }
  setMax();
    strip.setPixelColor(max_, WHITE);
  strip.show();
}

void setMax(){
  if (signal>max_ && signal < PIXELS){
    max_ = signal+1;
  } else if (signal == PIXELS) {
    max_ = PIXELS;
  } else {
    max_ -= 0.01;
    if (signal>max_) {
     max_ = signal+1;
    }  
  }
  
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } 
  else if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } 
  else {
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

uint32_t getColor(int i) {
  if (i<10) return GREEN;
  if (i>=10 && i<20) return YELLOW;
  if (i>20) return RED;  
}

