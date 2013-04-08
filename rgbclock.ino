// Code collected and completed by Tobias Floery <tobias@floery.net>

// FastSPI Library from http://code.google.com/p/fastspi/
#include <FastSPI_LED.h>

// Adafruits RTClib from https://github.com/adafruit/RTClib
#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 RTC;

// Sometimes chipsets wire in a backwards sort of way
struct CRGB { unsigned char b; unsigned char r; unsigned char g; };
//struct CRGB { unsigned char r; unsigned char g; unsigned char b; };
struct CRGB *leds;

// Number of RGB Pixel for FastSPI
#define NUM_LEDS 60
// Datapin for FastSPI
#define PIN 4

#define FILTER_LEN 8
// Brightness level
int level=15;

// Brightness correction for Leds
unsigned char bright[] = {0, 2, 4, 7, 11, 18, 30, 40, 50, 65, 80,96,125,160, 200, 255};

void setup()
{
  // Setup FastSPI Lib
  FastSPI_LED.setLeds(NUM_LEDS);
  FastSPI_LED.setChipset(CFastSPI_LED::SPI_TM1804);
  FastSPI_LED.setPin(PIN);
  
  FastSPI_LED.init();
  FastSPI_LED.start();

  leds = (struct CRGB*)FastSPI_LED.getRGBData(); 

  // init RTClib  
  Wire.begin();
  RTC.begin();
  
  //Serial.begin(9600);
  
  if (!RTC.isrunning()) {
    //Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  
  // setup measurement for LDR
  digitalWrite(A0, HIGH);  // set pullup on analog pin 0 
  pinMode(A1, OUTPUT);
  digitalWrite(A1, LOW);
}

unsigned int ldr=0;

void doLDR() {
  unsigned int ldr_value = analogRead(0);          //reads the LDR values
  
  ldr = ((unsigned long)(FILTER_LEN-1)*ldr+ldr_value)/FILTER_LEN;
  //Serial.println(ldr_value);

  unsigned int light_level=0;
  light_level = (1023-ldr)>>6;
  if (light_level >= 15) light_level = 15;
  if (light_level <= 1) light_level = 1;
  
  level = light_level;
  //Serial.println(light_level);
  //level=2;  
}

DateTime old;

void loop() { 
  // get time
  DateTime now = RTC.now();

  if (now.second()!=old.second()) {   
    old = now;
    doLDR();
  }

  // clear LED array
  memset(leds, 0, NUM_LEDS * 3);
    
  if (level < 4) {  
    // this is the night mode
    // set LED background
    for (int i=0; i<NUM_LEDS; i++) {
      leds[i].r = bright[level];
      leds[i].g = (bright[level]+1)/2;
    }  
    // set pixels
    //leds[(now.second())%60].r = bright[level];
    leds[(now.minute())%60].g = 0; 
    leds[(now.minute())%60].r = bright[level+2];
    
    unsigned char hourpos = (now.hour()%12)*5;
    leds[((59+hourpos)%60)%60].g = 0;
    leds[(hourpos)%60].g = 0;
    leds[((hourpos+1))%60].g = 0;
    
    leds[(hourpos)%60].r = bright[level+2];
    
  } else {

  
    // set pixels
    leds[(now.second())%60].r = bright[level];
    leds[(now.minute())%60].b = bright[level]; 
    
    unsigned char hourpos = (now.hour()%12)*5;
    leds[((59+hourpos)%60)%60].g = bright[level/2];
    leds[(hourpos)%60].g = bright[level];
    leds[((hourpos+1))%60].g = bright[level/2];  
  }
  // update LEDs
  FastSPI_LED.show();
  
  // wait some time
  delay(500);
}

