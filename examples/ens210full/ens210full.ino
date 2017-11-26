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

  // Enable I2C
  Wire.begin(D2,D1); // For ESP8266 NodeMCU boards [VDD to 3V3, GND to GND, SDA to D2, SCL to D1]
  // Wire.begin(); // Arduino pro mini or Nano [VDD to VCC/3V3, GND to GND, SDA to A4, SCL to A5]

  // Enable ENS210
  bool ok= ens210.begin();
  Serial.println(ok ? "ENS210 initialized" : "ERROR initializing ENS210");

  // Initialize measurement counter
  count= 0;
}


void loop() {
  // Execute a measurement
  int t_data, t_status, h_data, h_status;
  ens210.measure(&t_data, &t_status, &h_data, &h_status );

  // Print temperature
  if( t_status==ENS210_STATUS_OK ) {
    // Print raw, milli Kelvin, milli Celsius, milli Fahrenheit
    int32_t mK= ens210.toKelvin(t_data,1000);
    int32_t mC= ens210.toCelsius(t_data,1000);
    int32_t mF= ens210.toFahrenheit(t_data,1000);
    Serial.print((unsigned)t_data,HEX); Serial.print(" hexT, ");
    Serial.print(mK); Serial.print(" mK, ");
    Serial.print(mC); Serial.print(" mC, ");
    Serial.print(mF); Serial.print(" mF");
  } else {
    // Print fail status
    Serial.print("T-");
    Serial.print( ens210.status_str(t_status) );
  }

  // Print separator
  Serial.print("  -  ");

  // Print relative humidity
  if( h_status==ENS210_STATUS_OK ) {
    // Print raw, milli %RH
    int32_t mH= ens210.toPercentageH(h_data,1000);
    Serial.print((unsigned)h_data,HEX); Serial.print(" hexH, ");
    Serial.print(mH); Serial.print(" m%RH");
  } else {
    // Print fail status
    Serial.print("H-");
    Serial.print( ens210.status_str(h_status) );
  }

  // Print counter
  count++;
  Serial.print("  -  ");
  Serial.print(count);
  Serial.println();

  // Wait before next measurement
  delay(2000);
}