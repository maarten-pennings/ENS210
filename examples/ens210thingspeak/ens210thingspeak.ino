/*
  ens210thingspeak.ino - Upload ENS210 (temperature and humidity) measurements to a ThingSpeak channel from an ESP8266.
  Created by Maarten Pennings 2017 Dec 27
*/

/*
This script assumes you have
- installed the ENS210 Arduino library 
   Goto https://github.com/maarten-pennings/ENS210, press Download zipfile
   Click Sketch > Include Library > Add .ZIP Library...  then select downloaded zip file
- installed the ThingSpeak Arduino library 
   Sketch > Include Library > Manage Libraries > ThingSpeak > Install
- a ThingSpeak account
- a channel for ENS210 measurements
- created 4 fields for that channel:
   + Field 1: Temperature (°C)
   + Field 2: Humidity (%RH)
   + Field 3: TStatus
   + Field 4: HStatus
- used the "Channel ID" of that channel as initializer for thingspeakChannelId (see below)
- used the "Write API Key" of that channel as initializer for thingspeakWriteApiKey (see below)
*/


#include <Wire.h>        // I2C library
#include <ESP8266WiFi.h> // ESP8266 WiFi library
#include "ThingSpeak.h"  // ThingSpeak library
#include "ens210.h"      // ENS210 library


#define LED_PIN    D4    // GPIO2 == D4 == standard BLUE led available on most NodeMCU boards (LED on == D4 low)
#define led_init() pinMode(LED_PIN, OUTPUT)
#define led_on()   digitalWrite(LED_PIN, LOW)
#define led_off()  digitalWrite(LED_PIN, HIGH)
#define led_tgl()  digitalWrite(LED_PIN, (HIGH+LOW)-digitalRead(LED_PIN) );


#if 0
  // Fill out the credentials of your local WiFi Access Point
  const char *  wifiSsid              = "xxxxx"; // Your WiFi network SSID name
  const char *  wifiPassword          = "xxxxx"; // Your WiFi network password
  // Fill out the credentials of your ThingSpeak channel
  unsigned long thingspeakChannelId   = 1234567; // Your ThingSpeak Channel ID 
  const char *  thingspeakWriteApiKey = "xxxxx"; // Your ThingSpeak write api key
#else
  // File that contains (my secret) credentials for WiFi and ThingSpeak
  #include "credentials.h"
#endif


WiFiClient  client;
ENS210      ens210;


// Wrapper calling ens210.measure. It updates the arguments when the data was successfully read.
// Converts to °C and %RH
void ens210_measure(float * TData, int * TStatus, float * HData, int * HStatus) {
  // Read the ENS210
  int t_data, t_status, h_data, h_status;
  ens210.measure(&t_data, &t_status, &h_data, &h_status );

  // Use new values for T
  if( t_status==ENS210_STATUS_OK ) { // T data ok, so update
    *TData= ens210.toCelsius(t_data,1000)/1000.0;
    //*TData= ens210.toFahrenheit(t_data,1000)/1000.0;
    //*TData= ens210.toFahrenheit(t_data,1000)/1000.0;
    *TStatus = t_status;
  } else { // T data not ok, so do not update (keep old)
    *TStatus = t_status;
  }

  // Use new values for H
  if( h_status==ENS210_STATUS_OK ) { // H data ok, so update
    *HData= ens210.toPercentageH(h_data,1000)/1000.0;
    *HStatus = h_status;
  } else { // H data not ok, so do not update (keep old)
    *HStatus = h_status;
  }
}


void setup() {
  // Enable serial
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Starting ENS210 to ThingSpeak");

  // Enable LED
  led_init();
  Serial.println("init: LED up");
  led_on();

  // Enable I2C
  Wire.begin( /*SDA*/D2,/*SCL*/D1); // For ESP8266 NodeMCU boards
  // Wire.begin(); // Arduino pro mini or Nano
  Serial.println("init: I2C up");

  // Enable ENS210
  bool ok= ens210.begin();
  Serial.println(ok ? "init: ENS210 up" : "init: ENS210 ERROR");

  // Enable WiFi
  Serial.print("init: WiFi '");
  Serial.print(wifiSsid);
  Serial.print("' ");
  WiFi.begin(wifiSsid, wifiPassword);
  while( true) {
    if( WiFi.status()==WL_CONNECTED ) break;
    led_tgl();
    delay(250);
    Serial.printf(".");
  }
  Serial.printf(" up (%s)\n",WiFi.localIP().toString().c_str());

  // Enable ThingSpeak
  ThingSpeak.begin(client);
  Serial.println("init: ThingSpeak up");
  
  led_off();
}


float TData = 25.0; // °C
int   TStatus;
float HData = 50.0; // %RH
int   HStatus;


void loop() {
  // Signal start-of-measurement
  led_on();
  
  // Read the ENS210
  ens210_measure(&TData,&TStatus,&HData,&HStatus);
  
  // Print
  Serial.print("data: ");
  Serial.print("T="); Serial.print(TData,2);                    Serial.print("°C "); 
  Serial.print("(");  Serial.print(ens210.status_str(TStatus)); Serial.print(") "); 
  Serial.print("H="); Serial.print(HData,2);                    Serial.print("%RH "); 
  Serial.print("(");  Serial.print(ens210.status_str(HStatus)); Serial.print(") "); 
  Serial.println();
  
  // Send to ThingSpeak
  ThingSpeak.setField(1,TData);
  ThingSpeak.setField(2,HData);
  ThingSpeak.setField(3,TStatus);
  ThingSpeak.setField(4,HStatus);
  ThingSpeak.writeFields(thingspeakChannelId, thingspeakWriteApiKey);  

  // Signal end-of-measurement
  led_off();
  
  // Do not overfeed ThingSpeak
  delay(60000); 
}
