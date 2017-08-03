/*
  ens210.h - Library for the ENS210 relative humidity and temperature sensor with I2C interface from ams
  Created by Maarten Pennings 2017 Aug 1
*/
#ifndef __ENS210_H_
#define __ENS210_H_


#include <stdint.h>


#define ENS210_STATUS_OK        1
#define ENS210_STATUS_INVALID   2
#define ENS210_STATUS_CRCERROR  3
#define ENS210_STATUS_I2CERROR  4


class ENS210 {
  public: // Main API functions
    // Resets ENS210 and checks its PART_ID. Returns false on I2C problems or wrong PART_ID.
    // Optionally pass a solder correction (units: 1/64K, default is 50mK).
    // See "Effect of Soldering on Temperature Readout" in "Design-Guidelines"
    // https://download.ams.com/ENVIRONMENTAL-SENSORS/ENS210/Documentation
    bool begin(int correction=50*64/1000);                                
    // Performs one single shot temperature and relative humidity measurement. 
    // Sets t_data (temperature in 1/64K), and t_status (from ENS210STATUS_XXX).
    // Sets h_data (relative humidity in 1/512 %RH), and h_status (from ENS210STATUS_XXX).
    // Use the conversion functions below to convert t_data to K, C, F; or h_data to %RH.
    // Note that this function contains a delay of 130ms to wait for the measurement to complete.
    void measure(int * t_data, int * t_status, int * h_data, int * h_status ); 

  public: // Conversion functions - the temperature conversions also apply the solder correction (see begin() method).
    int32_t toKelvin     (int t_data, int multiplier); // Converts t_data (from `measure`) to multiplier*Kelvin
    int32_t toCelsius    (int t_data, int multiplier); // Converts t_data (from `measure`) to multiplier*Celsius
    int32_t toFahrenheit (int t_data, int multiplier); // Converts t_data (from `measure`) to multiplier*Fahrenheit
    int32_t toPercentageH(int h_data, int multiplier); // Converts t_data (from `measure`) to multiplier*%RH

  public: // Helpers function
    bool reset(void);                                  // Sends a reset to the ENS210. Returns false on I2C problems.
    bool lowpower(bool enable);                        // Sets ENS210 to low (true) or high (false) power. Returns false on I2C problems.
    bool getversion(uint16_t*partid,uint64_t*uid);     // Reads PART_ID and UID of ENS210. Returns false on I2C problems.
    bool startsingle(void);                            // Configures ENS210 to perform one single shot measurement. Returns false on I2C problems.
    bool read(uint32_t*t_val,uint32_t*h_val);          // Reads measurement data from the ENS210. Returns false on I2C problems.
    void extract(uint32_t val,int*data,int*status);    // Extracts measurement `data` and `status` from a `val` obtained from `read`.
    const char * status_str( int status );             // Converts a status (ENS210_STATUS_XXX) to a human readable string.

  private:
    int _soldercorrection;                             // Correction due to soldering (in 1/64K) subtracted from t_val by conversion functions.
};


#endif



