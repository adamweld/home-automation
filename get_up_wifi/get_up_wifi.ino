#include <HX711_ADC.h>
#include <EEPROM.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"
#include <WiFiUdp.h>
//#include "DebugMacros.h"

//HX711 constructor (dout pin, sck pin):
HX711_ADC LoadCell(11, 12);

const char *ssid     = "Bermuda Triangle 2.4G";
const char *password = "itsgonemissing";

const long utcOffsetInSeconds = -28800;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

const char* host = "script.google.com";
// Replace with your own script id to make server side changes
const char *GScriptId = "AKfycbxF_aZdoM-s5w1qN67l_0lzChXCC9s8qj1AIH9E5dWPtsreypk3";

const int httpsPort = 443;

// echo | openssl s_client -connect script.google.com:443 |& openssl x509 -fingerprint -noout
const char* fingerprint = "";
//const uint8_t fingerprint[20] = {};

// event request url
String url = String("/macros/s/") + GScriptId + "/exec?cal";

// verbose url
String url2 = String("/macros/s/") + GScriptId + "/exec?verbose";

HTTPSRedirect* client = nullptr;
// used to store the values of free stack and heap
// before the HTTPSRedirect object is instantiated
// so that they can be written to Google sheets
// upon instantiation

String payload = "{\"POST: Startup Message Received\"}";

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);


long t;
float load;

void setup(){
  float calValue = 5000;
  long tareValue = 138139083;

  Serial.begin(115200);

  WiFi.begin(ssid, password);

  LoadCell.begin();
  int stabilisingtime = 1; // tare preciscion can be improved by adding a few seconds of stabilising time
  LoadCell.start(stabilisingtime);
  if(LoadCell.getTareTimeoutFlag()) {
    Serial.println("Tare timeout, check MCU>HX711 wiring and pin designations");
  }
  else {
    LoadCell.setCalFactor(calValue); // set calibration value (float)
    LoadCell.setTareOffset(tareValue);
    Serial.println("Startup + tare is complete\n\n");
  }

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());  
  Serial.print("Connecting to ");
  Serial.println(host);

  
  timeClient.begin();

  
  Serial.println("\nEnter Main");
  Serial.println("================================");
}

void loop() {
  static int error_count = 0;
  static int connect_count = 0;
  const unsigned int MAX_CONNECT = 20;
  static bool flag = false;

  int alarm = 0;
  
  timeClient.update();
  LoadCell.update();

  //get smoothed value from data set
  if (millis() > t + 250) {
    load = LoadCell.getData();
    Serial.print("Time:   " + timeClient.getFormattedTime());
//    Serial.print("     Load:   " + "     Alarm:   ");
    Serial.println(alarm);
    t = millis();
  }

  // update from cal every 10 seconds
  if(timeClient.getSeconds() & 10 == 0) {
  
    // initial redirect call
    if (!flag){
      client = new HTTPSRedirect(httpsPort);
      client->setInsecure();
      flag = true;
      client->setPrintResponseBody(true);
      client->setContentTypeHeader("application/json");
    }
    // start with POST that does nothing
    if (client != nullptr){
      if (!client->connected()){
        client->connect(host, httpsPort);
        client->POST(url, host, payload);
      }
    }
  
    // max number of connections before we restard with new redirect object for stability
    if (connect_count > MAX_CONNECT){
      //error_count = 5;
      connect_count = 0;
      flag = false;
      delete client;
      return;
    }
  
    // update alarm status
    client->GET(url, host);
    String response = client->getResponseBody();
    Serial.println("response: " + response);
    alarm = response.startsWith("t");
    ++connect_count;
  }
}

// print time() day and current time
void printTime() {
  Serial.print(daysOfTheWeek[timeClient.getDay()]);
  Serial.print(", ");
  Serial.print(timeClient.getHours());
  Serial.print(":");
  Serial.print(timeClient.getMinutes());
  Serial.print(":");
  Serial.println(timeClient.getSeconds());
  //Serial.println(timeClient.getFormattedTime());
}
