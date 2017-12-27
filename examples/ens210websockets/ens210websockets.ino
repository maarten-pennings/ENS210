/*
  ens210websockets.ino - Web (and websocket) server for ESP8266 that streams ENS210 data (temperature and humidity) to any web browser.
  Created by Maarten Pennings 2017 Dec 27
*/

/*
This sketch assumes you have
- an ESP8266 with an ENS210 connected
- the Arduino IDE
- installed the ENS210 Arduino library 
   Goto https://github.com/maarten-pennings/ENS210, press Download zipfile
   Click Sketch > Include Library > Add .ZIP Library...  then select downloaded zip file
- installed the WebSockets and DNS libraries
   Sketch > Include Library > Manage Libraries > WebSockets or DNS > Install
- installed to flash file system uploader in Arduino IDE
   Goto http://esp8266.github.io/Arduino/versions/2.0.0/doc/filesystem.html#uploading-files-to-file-system
- uploaded the data/index.html file to the flash file system on the ESP8266
   Tool > ESP8266 Sketch Data Upload 
- compiled, uploaded and started the sketch

How to use
 - This app creates an access point.
 - From a laptop or smartphone connect to this access point.
 - On the laptop or accesspoint startup a browser and visit any page (e.g. 10.10.10.10).
 - A webpage will popup showing a stream of ENS210 measurements.
 - Note that this app prints a log to serial.
*/


#include <Wire.h>             // I2C library
#include <WebSocketsServer.h> // Websocket server library
#include <ESP8266WebServer.h> // HTTP server library
#include <DNSServer.h>        // DNS server library
#include <FS.h>               // Flash file system library
#include "ens210.h"           // ENS210 library


#define APNAME  "ENS210WS"    // Name of the Access Point
#define FILE    "index.html"  // The file to returns to webbrowser 
#define WAIT    500           // Delays in ms between websocket messages (ens210 measurements)


#define LED_PIN    D4    // GPIO2 == D4 == standard BLUE led available on most NodeMCU boards (LED on == D4 low)
#define led_init() pinMode(LED_PIN, OUTPUT)
#define led_on()   digitalWrite(LED_PIN, LOW)
#define led_off()  digitalWrite(LED_PIN, HIGH)
#define led_tgl()  digitalWrite(LED_PIN, (HIGH+LOW)-digitalRead(LED_PIN) );


WebSocketsServer ws_server(81);
ESP8266WebServer http_server(80);
DNSServer        dns_server;
ENS210           ens210;


void ws_event(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  (void)payload;
  (void)length;
  switch( type ) {
    case WStype_CONNECTED   : Serial.printf("wsev: connected (slot %u)\n", num); break;
    case WStype_DISCONNECTED: Serial.printf("wsev: disconnected (slot %u)\n", num); break;
    default                 : Serial.printf("wsev: other %d (slot %u)\n", type, num); break;
  }
} 


// Wrapper calling ens210.measure. It updates the arguments when the data was successfully read. Converts to °C and %RH
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
  bool ok;
  
  // Enable Serial
  Serial.begin(115200);
  Serial.printf("\n\ninit: Welcome to ENS210 websockets\n");

  // Enable LED
  led_init();
  Serial.println("init: LED up");
  led_on();

  // Enable I2C
  Wire.begin( /*SDA*/D2,/*SCL*/D1); 
  Serial.println("init: I2C up");

  // Enable ENS210
  ok= ens210.begin();
  Serial.printf("init: ENS210 %s\n", ok ? "up" : "ERROR");

  // Enable flash file system
  ok= SPIFFS.begin();
  Serial.printf("init: FFS %s\n", ok ? "up" : "ERROR");

  // Start access point
  IPAddress ip(10, 10, 10, 10);
  WiFi.hostname(APNAME); 
  WiFi.softAPConfig(ip, ip, IPAddress(255, 255, 255, 0));
  Serial.printf("init: AP config '%s' at %s\n", APNAME, ip.toString().c_str());
  WiFi.mode(WIFI_AP);
  ok= WiFi.softAP(APNAME);
  Serial.printf("init: AP %s\n", ok ? "up" : "ERROR");

  // Start dns on standard port (53) 
  dns_server.start(53, "*", ip);
  Serial.println("init: DNS server up");

  // Start websocket server
  ws_server.begin();
  ws_server.onEvent(ws_event);
  Serial.printf("init: WS server up\n");

  // Configure and start http server 
  http_server.on("/", []() {
    Serial.printf("http: req '/' ... ");
    File f= SPIFFS.open("/" FILE, "r");
    Serial.printf("sending '%s' size %d ... ", FILE, f.size());
    http_server.streamFile(f, "text/html");
    f.close();
    Serial.printf("done\n");
  });
  http_server.begin();
  Serial.printf("init: HTTP server up\n");

  // Setup done
  Serial.printf("\n");
  led_off();
}


float    TData = 25.0; // °C
int      TStatus;
float    HData = 50.0; // %RH
int      HStatus;


void loop() {
  // Let all servers process requests
  ws_server.loop();
  http_server.handleClient();
  dns_server.processNextRequest();

  // Abort loop() if too early for next measurement
  uint32_t now= millis();
  static uint32_t prev;
  uint32_t wait= now-prev;
  if( wait<WAIT ) return;
  prev= now;

  // Read and broadcast ENS210 measurement
  led_on();
  ens210_measure(&TData,&TStatus,&HData,&HStatus);
  String T= String(TData,2);
  String H= String(HData,2);
  Serial.printf("data: %s°C (%s) %s%%RH (%s) %ums\n",T.c_str(),ens210.status_str(TStatus),H.c_str(),ens210.status_str(HStatus),wait);
  ws_server.broadcastTXT(String("T=")+T+", H="+H);
  led_off();  
}

