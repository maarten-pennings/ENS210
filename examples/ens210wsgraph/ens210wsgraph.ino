/*
  ens210wsgraph.ino - Streams ENS210 data (temperature and humidity) from an ESP8266 via websockets to a graph in a web browser.
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
 - This app creates an access point (named ENS210).
 - From a laptop or smartphone connect to this access point.
 - On the laptop or accesspoint startup a browser and visit any page (e.g. 10.10.10.10).
 - A webpage will popup showing a moving graph of ENS210 measurements.
 - Note that this app prints a log to serial.
 - Note that the javascript in the browser also prints a log to console.
*/


#include <Wire.h>             // I2C library
#include <WebSocketsServer.h> // Websocket server library
#include <ESP8266WebServer.h> // HTTP server library
#include <DNSServer.h>        // DNS server library
#include <FS.h>               // Flash file system library
#include "ens210.h"           // ENS210 library


#define APNAME   "ENS210"     // Name of the Access Point
#define FILEHTML "index.html" // The html file to return to webbrowsers
#define FILEJS   "smoothie.js"// The js file to return to webbrowsers
#define VERSION  "v1.1"       // Version of this app


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
  (void)payload; (void)length; // Suppress warnings
  switch( type ) {
    case WStype_CONNECTED   : Serial.printf("wsev: connected (slot %u)\n", num); break;
    case WStype_DISCONNECTED: Serial.printf("wsev: disconnected (slot %u)\n", num); break;
    default                 : Serial.printf("wsev: other %d (slot %u)\n", type, num); break;
  }
} 


// Wrapper calling ens210.measure. It updates the arguments when the data was successfully read. Converts to °C and %RH
void ens210_measure(float * TData, int * TStatus, float * HData, int * HStatus) {
  // Read the ENS210 (into temporary variables)
  int _TData, _HData;
  ens210.measure(&_TData, TStatus, &_HData, HStatus );
  // Update TData and HData when there was no error
  if( *TStatus==ENS210_STATUS_OK ) *TData= ens210.toCelsius(_TData,1000)/1000.0;     
  if( *HStatus==ENS210_STATUS_OK ) *HData= ens210.toPercentageH(_HData,1000)/1000.0; 
}


void setup() {
  bool ok;
  
  // Enable Serial
  Serial.begin(115200);
  Serial.printf("\n\ninit: Welcome to ENS210 websockets %s\n", VERSION);

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
  dns_server.setErrorReplyCode(DNSReplyCode::NoError);
  dns_server.start(53, "*", ip);
  Serial.println("init: DNS server up");

  // Start websocket server
  ws_server.begin();
  ws_server.onEvent(ws_event);
  Serial.printf("init: WS server up\n");

  // Configure and start http server 
  http_server.on("/", []() {
    Serial.printf("http: req '/' ... ");
    File f= SPIFFS.open("/" FILEHTML, "r");
    Serial.printf("sending '%s' size %d ... ", FILEHTML, f.size());
    http_server.streamFile(f, "text/html");
    f.close();
    Serial.printf("done\n");
  });
  http_server.on("/" FILEJS, []() {
    Serial.printf("http: req '/" FILEJS "' ... ");
    File f= SPIFFS.open("/" FILEJS, "r");
    Serial.printf("sending '%s' size %d ... ", FILEJS, f.size());
    http_server.streamFile(f, "application/javascrip");
    f.close();
    Serial.printf("done\n");
  });
  http_server.onNotFound( []() {
    Serial.printf("http: req '%s' ... ",http_server.uri().c_str());
    http_server.sendHeader("Location", "http://10.10.10.10/", true);
    http_server.send( 302, "text/plain", "");
    Serial.printf("redirected\n");
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

  // Read and broadcast ENS210 measurement
  ens210_measure(&TData,&TStatus,&HData,&HStatus);
  String T= String(TData,2);
  String H= String(HData,2);
  Serial.printf("data: %s°C (%s) %s%%RH (%s)\n",T.c_str(),ens210.status_str(TStatus),H.c_str(),ens210.status_str(HStatus));
  ws_server.broadcastTXT(String("T=")+T+", H="+H);

  // Led feedback
  led_tgl();
}

