/*
  ens210cont.ino - Demo sketch for continuous mode of the ENS210 relative humidity and temperature sensor with I2C interface from ams (no error handling)
  Created by Maarten Pennings 2018 oct 23
*/


#include <Wire.h>   // I2C library
#include "ens210.h" // ENS210 library


ENS210 ens210;


void setup() {
  // Enable serial
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Starting ENS210 continuous mode demo");

  // Enable I2C
  Wire.begin(); // For ESP8266 NodeMCU boards: VDD to 3V3, GND to GND, SDA to D2, SCL to D1

  // Enable ENS210
  ens210.begin();

  // Start continuous mode
  ens210.startcont();

  // Wait till first measurement is ready (comment out, and see status INVALID)
  delay(ENS210_THCONV_CONT_MS);
}


void loop() {
  // Get the measurement data  
  int t_data, t_status, h_data, h_status;
  ens210.read(&t_data, &t_status, &h_data, &h_status );

  // Print the results
  Serial.print(ens210.status_str(t_status));        Serial.print(" "); 
  Serial.print(ens210.toCelsius(t_data,10)/10.0,1); Serial.print(" C, ");
  Serial.print(ens210.status_str(h_status));        Serial.print(" "); 
  Serial.print(ens210.toPercentageH(h_data,1));     Serial.print(" %RH");
  Serial.println();

  // Uncomment for a test: stop measuring after 15 measurements
  // static int count; if( count++==15 ) { Serial.println("stopped"); ens210.stopcont(); } 

  // Arbitrary delay  
  delay(2000);
}

