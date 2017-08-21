/*
  ens210simple.ino - Simple demo sketch (no error handling) printing results of the ENS210 relative humidity and temperature sensor with I2C interface from ams
  Created by Maarten Pennings 2017 Aug 2
*/


#include <Wire.h>   // I2C library
#include "ens210.h" // ENS210 library


ENS210 ens210;


void setup() {
  // Enable serial
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Starting ENS210 simple demo");

  // Enable I2C
  // Wire.begin(D2,D1); // For ESP8266 nodemcu boards [VDD to 3V3, GND to GND, SDA to D2, SCL to D1]
  Wire.begin(); // Arduino pro mini [VDD to VCC, GND to GND, SDA to A4, SCL to A5]

  // Enable ENS210
  ens210.begin();
}


void loop() {
  int t_data, t_status, h_data, h_status;
  ens210.measure(&t_data, &t_status, &h_data, &h_status );

  Serial.print( ens210.toCelsius(t_data,10)/10.0, 1 ); Serial.print(" C, ");
  Serial.print( ens210.toPercentageH(h_data,1)      ); Serial.print(" %RH");
  Serial.println();

  delay(2000);
}

