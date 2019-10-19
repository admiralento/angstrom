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

//Set Defintions
#define NETWORKID     0   // Must be the same for all nodes
#define MYNODEID      1   // My node ID
#define TONODEID      2   // Destination node ID
#define FREQUENCY     RF69_915MHZ

RFM69 radio;

// Create constants here
#define MISOPIN      7
#define MOSIPIN      6
#define SCKPIN       5
#define NSSPIN       4
#define INTPIN       3
#define SCLPIN       13
#define SDAPIN       12

//Variables
char sendBuffer[62];
int bufferIndex = 0;

void setup() {

  //Init radio communication
  radio.initialize(FREQUENCY, MYNODEID, NETWORKID);
  radio.setHighPower();
  radio.setPowerLevel(20);
  radio.encrypt(0);

  Serial.begin(9600);  //Initialize Serial Communication

}

void loop() {

  //Fetch Data from IMU
  //Save to buffer
  sendBuffer[bufferIndex] = IMU.read();
  bufferIndex++;

  //Fetch Data from Atmospheric Sensor
  //Save to buffer
  sendBuffer[bufferIndex] = BME.read();
  bufferIndex++;

  //If enough data saved in buffer
  //Send data packet by radio
  //and write to memory module
  if (bufferIndex == 61) {
    radio.send(TONODEID, sendbuffer, bufferIndex);
    bufferIndex = 0;
  }

  //Repeat

}
