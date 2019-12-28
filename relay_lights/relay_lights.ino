/*
    adam weld 2019
    home automation sketch
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>
#include <functional>
#include "switch.h"
#include "UpnpBroadcastResponder.h"
#include "CallbackFunction.h"
#include "Bounce2.h"

// set wifi network username/pw
#ifndef STASSID
#define STASSID "Bermuda Triangle 2.4G"
#define STAPSK  "itsgonemissing"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

// Set Relay Pins
const int relayOne = 9;
const int relayTwo = 10;
const int relayThree = 5;
const int relayFour = 4;

const int npxPin = 13;

const int buttonPin = 14;
Bounce debouncer = Bounce();
int godIsReal = 0;

//on/off callbacks
void relayOneOn();
void relayOneOff();
void relayTwoOn();
void relayTwoOff();
void relayThreeOn();
void relayThreeOff();
void relayFourOn();
void relayFourOff();

// upnp object
UpnpBroadcastResponder upnpBroadcastResponder;

// neopixel object
Adafruit_NeoPixel strip = Adafruit_NeoPixel(200, npxPin, NEO_GRB + NEO_KHZ800);

int pixel_on = 0;

// define switches
Switch *lightOne = NULL;
Switch *lightTwo = NULL;
Switch *lightThree = NULL;
Switch *lightFour = NULL;
Switch *lightNpx = NULL;


void setup() {
  Serial.begin(115200);

  debouncer.attach(buttonPin, INPUT_PULLUP);
  debouncer.interval(25);
  
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  // set relay pin outputs
  pinMode(relayOne, OUTPUT);
  pinMode(relayTwo, OUTPUT);
  pinMode(relayThree, OUTPUT);
  pinMode(relayFour, OUTPUT);

  // set up neopixel strip
  strip.begin();
  strip.setBrightness(50);
  strip.show(); // Initialize all pixels to 'off'
  Serial.println("");
  Serial.println("NeoPixels Initialized");

  // set initial conditions (off)
  relayOneOff();
  relayTwoOff();
  relayThreeOff();
  relayFourOff();

//  relayOneOn();
//  relayTwoOn();
//  relayThreeOn();
//  relayFourOn();
//  npxOff();

  Serial.println("");
  Serial.println("GPIO Initialized");

  // Define switches here. Max 14
  // Format: Alexa invocation name, local port no, on callback, off callback
  lightOne = new Switch("Relay One", 80, relayOneOn, relayOneOff);
  lightTwo = new Switch("Lamp", 81, relayTwoOn, relayTwoOff);
  lightThree = new Switch("Relay Three", 82, relayThreeOn, relayThreeOff);
  lightFour = new Switch("God", 83, relayFourOn, relayFourOff);
  lightNpx = new Switch("Mood Light", 84, npxOn, npxOff);
  

  Serial.println("Adding switches upnp broadcast responder");
  upnpBroadcastResponder.beginUdpMulticast();
  upnpBroadcastResponder.addDevice(*lightOne);
  upnpBroadcastResponder.addDevice(*lightTwo);
  upnpBroadcastResponder.addDevice(*lightThree);
  upnpBroadcastResponder.addDevice(*lightFour);
  upnpBroadcastResponder.addDevice(*lightNpx);

  for(uint16_t i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, 128);
  }
  strip.show();
}

void loop() {
  debouncer.update();
  
  upnpBroadcastResponder.serverLoop();
  lightOne->serverLoop();
  lightTwo->serverLoop();
  lightThree->serverLoop();
  lightFour->serverLoop();
  lightNpx->serverLoop();

  // do neopixel loop
//  if(pixel_on) {
//    rainbowCycle(20);
//  }

  if (debouncer.fell()) {  // Call code if button transitions from HIGH to LOW
    Serial.println("fell");
    if(godIsReal) {
      relayFourOff();
    } else {
      relayFourOn();
    }
    godIsReal = !godIsReal;
  }
}

// device functions
void relayOneOn() {
  Serial.print("relay one turn on ...");
  digitalWrite(relayOne, LOW);   // sets relayOne on
}
void relayOneOff() {
  Serial.print("relay one turn off ...");
  digitalWrite(relayOne, HIGH);   // sets relayOne off
}
void relayTwoOn() {
  Serial.print("relay Two turn on ...");
  digitalWrite(relayTwo, LOW);   // sets relayTwo on
}
void relayTwoOff() {
  Serial.print("relay Two turn off ...");
  digitalWrite(relayTwo, HIGH);   // sets relayTwo off
}
void relayThreeOn() {
  Serial.print("relay Three turn on ...");
  digitalWrite(relayThree, LOW);   // sets relayThree on
}
void relayThreeOff() {
  Serial.print("relay Three turn off ...");
  digitalWrite(relayThree, HIGH);   // sets relayThree off
}
void relayFourOn() {
  Serial.print("relay Four turn on ...");
  digitalWrite(relayFour, LOW);   // sets relayFour on
}
void relayFourOff() {
  Serial.print("relay Four turn off ...");
  digitalWrite(relayFour, HIGH);   // sets relayFour off
}

void npxOn() {
//  rainbowCycle(20);
  pixel_on = 1;
}

void npxOff() {
  for (uint16_t i=0; i < strip.numPixels(); i=i++) {
    strip.setPixelColor(i, 0);    //turn every third pixel on
  }
  strip.show();
  pixel_on = 0;
}


// NeoPixel functions

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
