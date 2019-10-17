/*
 *    Project Angstrom
 *    Femtosat Flight Software
 * 
 *    BYU
 *    Autumn 2019
 * 
 *    Austin Stone, David Chang, and Ammon Wolfert
 *    
 *    To install required libraries on your computer
 *    please move the folders from your Github folder 
 *    under "Arduino Libraries"
 *    into Documents --> Arduino --> libraries
 *    There should be a RFM69 and MPU9250 Library in there
 */

//include libraries
#include <SPI.h>
#include <RFM69.h>
#include <MPU9250.h>
#include <SparkFunBME280.h>


// Create constants here
const int misoPin = 7;
const int mosiPin = 6;
const int sckPin = 5;
const int nssPin = 4;
const int intPin = 3;
const int sclPin = 13;
const int sdaPin = 12;

void setup() {

  //set the pinmode of all communication pins
  //begin serial communication (for debugbing with computer)
  RFM69 radio;
  
  Serial.begin(9600);  //Initialize Serial Communication
  
}

void loop() {

  //Fetch Data from IMU
  //Save to buffer

  //Fetch Data from Atmospheric Sensor
  //Save to buffer

  //If enough data saved in buffer
  //Send data packet by radio
  //and write to memory module

  //Repeat
 
}
