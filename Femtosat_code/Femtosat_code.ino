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
#include <BME280.h>


// Create constants here
const int ledPin = 13;

unsigned long triggerTime = 0;
unsigned long timeInterval = 500; //Milliseconds LED will be lit and unlit (Half a Period)

bool ledState = LOW;

void setup() {


  //set the pinmode of all communication pins
  //begin serial communication (for debugbing with computer)

  pinMode(ledPin,OUTPUT);  //Set the LED pin to be an output
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

  if (millis() - triggerTime >= timeInterval) {   //Check if enough has passed
    ledState = !ledState;                         //Alternate State of Led
    digitalWrite(ledPin, ledState);               //Write to Led
    triggerTime = millis();                       //Reset Timer
  }
  

}
