/*
  ens210single.ino - Demo sketch for single shot mode, doing something else during the conversion (no error handling)
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
}


void loop() {
  // Start one single shot conversion
  ens210.startsingle();
  
  // Start other work during ENS210 conversion
  Serial.print("Working ");
  for( int i=0; i<10; i++ ) { 
    Serial.print("."); 
    delay(20); // Try delay of 10; read comes too soon and gives INVALID
  }
  Serial.println(" done");

  // Get the measurement data  
  int t_data, t_status, h_data, h_status;
  ens210.read(&t_data, &t_status, &h_data, &h_status );

  // Print the results
  Serial.print(ens210.status_str(t_status));        Serial.print(" "); 
  Serial.print(ens210.toCelsius(t_data,10)/10.0,1); Serial.print(" C, ");
  Serial.print(ens210.status_str(h_status));        Serial.print(" "); 
  Serial.print(ens210.toPercentageH(h_data,1));     Serial.print(" %RH");
  Serial.println();

  // Arbitrary delay  
  delay(2000);
}

