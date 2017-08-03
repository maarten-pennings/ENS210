/*
  ens210full.ino - Full demo sketch (including error handling and all conversions) printing results of the ENS210 relative humidity and temperature sensor with I2C interface from ams
  Created by Maarten Pennings 2017 Aug 2
*/


#include <Wire.h>   // I2C library
#include "ens210.h" // ENS210 library


ENS210 ens210;
int    count;


void setup() {
  // Enable serial
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Starting ENS210 full demo");

  // Enable I2C, reset ENS210
  Wire.begin(/*SDA*/D2, /*SCL*/D1); // VDD to 3V3, GND to GND
  bool ok= ens210.begin();
  Serial.println(ok ? "ENS210 initialized" : "ERROR initializing ENS210");

  // Initialize measurement counter
  count= 0;
}


void loop() {
  int t_data, t_status, h_data, h_status; 
  ens210.measure(&t_data, &t_status, &h_data, &h_status ); 

  // Print temperature (raw, milli Kelvin, milli Celsius, milli Fahrenheit)
  int32_t mK= ens210.toKelvin(t_data,1000);
  int32_t mC= ens210.toCelsius(t_data,1000);
  int32_t mF= ens210.toFahrenheit(t_data,1000);
  Serial.print( ens210.status_str(t_status) ); Serial.print(" T, ");
  Serial.print(t_data,HEX); Serial.print(" hexT, ");
  Serial.print(mK); Serial.print(" mK, ");
  Serial.print(mC); Serial.print(" mC, ");
  Serial.print(mF); Serial.print(" mF");

  // Print separator
  Serial.print("  -  ");

  // Print relative humidity (raw, milli %RH)
  int32_t mH= ens210.toPercentageH(h_data,1000);
  Serial.print( ens210.status_str(h_status) ); Serial.print(" H, ");
  Serial.print(h_data,HEX); Serial.print(" hexH, ");
  Serial.print(mH); Serial.print(" m%RH");
  
  // Print counter
  count++;
  Serial.print("  -  ");
  Serial.print(count); 
  Serial.println();

  delay(2000);
}

