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

///////////////
//
//#define DEBUGGING       //Define if debugging, if not defined complier will ignore debugging code
//
//////////////

//include libraries
#include <SPI.h>
#include <RFM69.h>
#include <MPU9250.h>
#include <SparkFunBME280.h>
#include "FemtosatMem.h"

//Define Radio
#define NETWORKID     0   // Must be the same for all nodes
#define MYNODEID      1   // My node ID
#define TONODEID      2   // Destination node ID
#define FREQUENCY     RF69_915MHZ

//Define IMU
#define I2Cclock 400000
#define I2Cport Wire
#define MPU9250_ADDRESS MPU9250_ADDRESS_AD0

//Define Mem
#define WAIT_TIME 3 

//Define Objects
RFM69 radio;
MPU9250 myIMU(MPU9250_ADDRESS, I2Cport, I2Cclock);
BME280 myBME;
FemtosatMem mem;

//Memory File
char fileName[] = "ang.txt";

//Variables
#define MAX_PACKET_SIZE   62
#define DATA_POINTS       10

#define LED             3
#define BLINK_INTERVAL  100

uint8_t start_word[] = "begin";
uint8_t end_word[] = "end";
uint8_t sendBuffer[MAX_PACKET_SIZE];
float data[DATA_POINTS];

int sendLength = 0;
int dataIndex = 0;

//Functions
void collectDataIMU();
void copyInto(uint8_t list[], uint8_t thing[], int START, int SIZE);
void Blink();

#ifdef DEBUGGING
void doIMUSelfTest();
#endif

void setup() {

  Wire.begin();                                               //Init IC2 Communication
  
  #ifdef DEBUGGING
  Serial.begin(9600);                                         //Initialize Serial Communication
  while(!Serial){};                                           //Wait for Connection
  #endif

  radio.initialize(FREQUENCY, MYNODEID, NETWORKID);           //Init radio communication
  radio.setHighPower();
  radio.encrypt(0);

  #ifdef DEBUGGING                                            //Init the IMU
  doIMUSelfTest();
  #else
  myIMU.initMPU9250();
  myIMU.calibrateMPU9250(myIMU.gyroBias, myIMU.accelBias);
  #endif

  myBME.setI2CAddress(0x76);                                  //Init the BME
  #ifdef DEBUGGING
  if(myBME.beginI2C() == false) {
    Serial.println("BME connection failed.");
  } else {
    Serial.println("BME connection suceeded.");
  }
  #endif
  
  mem.begin(fileName, sizeof(fileName));                      //Init the Memory Module
}

void loop() {

  collectDataIMU();                                                                 //Collect all data from sensors
  data[0] = (float)millis();                                                        //Float 0 - - - - - TIME IN MILLISECONDS
  data[1] = myIMU.ax;                                                               //Float 1 - - - - - X-ACCELERATION
  data[2] = myIMU.ay;                                                               //Float 2 - - - - - Y-ACCELERATION
  data[3] = myIMU.az;                                                               //Float 3 - - - - - Z-ACCELERATION
  data[4] = myIMU.gx;                                                               //Float 4 - - - - - X-ROTATION
  data[5] = myIMU.gy;                                                               //Float 5 - - - - - Y-ROTATION
  data[6] = myIMU.gz;                                                               //Float 6 - - - - - Z-ROTATION
  data[7] = myBME.readFloatHumidity();                                              //Float 7 - - - - - HUMIDITY
  data[8] = myBME.readFloatPressure();                                              //Float 8 - - - - - PRESSURE
  data[9] = myBME.readTempF();                                                      //Float 9 - - - - - TEMPERATURE

  dataIndex = 0;

  copyInto(sendBuffer, start_word, sendLength, sizeof(start_word) - 1);           //Write a start transmission marker
  sendLength += sizeof(start_word) - 1;
  
  for (int i = 0; i < DATA_POINTS; i++) {
      uint8_t* bytes;
      bytes = (uint8_t*)(&data[dataIndex]);
      copyInto(sendBuffer, bytes, sendLength, 4);                                 //Write float data, 4 bytes a piece
      sendLength += 4;
      dataIndex++;
  }
  
  copyInto(sendBuffer, end_word, sendLength, sizeof(end_word) - 1);               //Write a end transmission marker
  sendLength += sizeof(end_word) - 1;

  #ifdef DEBUGGING
  Serial.print("Radio Transmission: [");
  for (int i = 0; i < sendLength; i++) {
      Serial.print(sendBuffer[i], HEX);
      if (i != sendLength - 1) {
          Serial.print(", ");
      } else {
          Serial.println("]");
      }
  }
  Serial.print("Data Recieved: [");
  for (int i = 0; i < DATA_POINTS; i++) {
      float* newDataPoint;
      newDataPoint = (float*)&sendBuffer[sizeof(start_word) - 1 + i*4];
      Serial.print(newDataPoint[0]);
      if (i != DATA_POINTS - 1) {
          Serial.print(", ");
      } else {
          Serial.println("]");
      }
  }
  #endif

  radio.send(TONODEID, sendBuffer, sendLength, false);                              //Send radio packet

  while(!mem.Ready()) { delay(WAIT_TIME); }                                         //Wait till memory module is ready
  mem.Save(sendBuffer, 24);                                                 //Save to memory module
  while(!mem.Ready()) { delay(WAIT_TIME); }                                         //Wait till memory module is ready
  mem.Save(sendBuffer + 24, 24);                                                 //Save to memory module

  Blink();
  
  sendLength = 0;

}

void copyInto(uint8_t list[], uint8_t thing[], int START, int SIZE) {
    for (int i = 0; i < SIZE; i++) {
        list[i + START] = thing[i];
    }
}

void collectDataIMU() {
  //Fetch Data from IMU
  myIMU.readAccelData(myIMU.accelCount);  // Read the x/y/z adc values

  // Now we'll calculate the accleration value into actual g's
  // This depends on scale being set
  myIMU.ax = (float)myIMU.accelCount[0] * myIMU.aRes; // - myIMU.accelBias[0];
  myIMU.ay = (float)myIMU.accelCount[1] * myIMU.aRes; // - myIMU.accelBias[1];
  myIMU.az = (float)myIMU.accelCount[2] * myIMU.aRes; // - myIMU.accelBias[2];

  myIMU.readGyroData(myIMU.gyroCount);  // Read the x/y/z adc values

  // Calculate the gyro value into actual degrees per second
  // This depends on scale being set
  myIMU.gx = (float)myIMU.gyroCount[0] * myIMU.gRes;
  myIMU.gy = (float)myIMU.gyroCount[1] * myIMU.gRes;
  myIMU.gz = (float)myIMU.gyroCount[2] * myIMU.gRes;
  
}

#ifdef DEBUGGING
void doIMUSelfTest(){
  //Makes sure the IMU is connected and calibrates it
  byte c = myIMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
  
  Serial.print("Attempting to read ");
  Serial.print(MPU9250_ADDRESS, HEX);
  Serial.print(" ");
  Serial.println(WHO_AM_I_MPU9250, HEX);
  Serial.print(F("MPU9250 I AM 0x"));
  Serial.print(c, HEX);
  Serial.print(F(" I should be 0x"));
  Serial.println(0x71, HEX);

  if (c == 0x71) // WHO_AM_I should always be 0x71
  {
    Serial.println(F("MPU9250 is online..."));

    // Start by performing self test and reporting values
    myIMU.MPU9250SelfTest(myIMU.selfTest);
    Serial.print(F("x-axis self test: acceleration trim within : "));
    Serial.print(myIMU.selfTest[0],1); Serial.println("% of factory value");
    Serial.print(F("y-axis self test: acceleration trim within : "));
    Serial.print(myIMU.selfTest[1],1); Serial.println("% of factory value");
    Serial.print(F("z-axis self test: acceleration trim within : "));
    Serial.print(myIMU.selfTest[2],1); Serial.println("% of factory value");
    Serial.print(F("x-axis self test: gyration trim within : "));
    Serial.print(myIMU.selfTest[3],1); Serial.println("% of factory value");
    Serial.print(F("y-axis self test: gyration trim within : "));
    Serial.print(myIMU.selfTest[4],1); Serial.println("% of factory value");
    Serial.print(F("z-axis self test: gyration trim within : "));
    Serial.print(myIMU.selfTest[5],1); Serial.println("% of factory value");

    myIMU.initMPU9250();
    myIMU.calibrateMPU9250(myIMU.gyroBias, myIMU.accelBias);

    Serial.println("MPU9250 initialized for active data mode....");
    // Read the WHO_AM_I register of the magnetometer, this is a good test of
    // communication
    byte d = myIMU.readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);
    Serial.print("AK8963 ");
    Serial.print("I AM 0x");
    Serial.print(d, HEX);
    Serial.print(" I should be 0x");
    Serial.println(0x48, HEX);

    myIMU.getAres();
    myIMU.getGres();
    myIMU.getMres();

    Serial.println("AK8963 mag biases (mG)");
    Serial.println(myIMU.magBias[0]);
    Serial.println(myIMU.magBias[1]);
    Serial.println(myIMU.magBias[2]);

    Serial.println("AK8963 mag scale (mG)");
    Serial.println(myIMU.magScale[0]);
    Serial.println(myIMU.magScale[1]);
    Serial.println(myIMU.magScale[2]);
  } else
  {
    Serial.print("Could not connect to MPU9250: 0x");
    Serial.println(c, HEX);

    // Communication failed, stop here
    Serial.println(F("Communication failed, abort!"));
    Serial.flush();
    abort();
  }
}
#endif

void Blink() {
  digitalWrite(LED, HIGH);
  delay(BLINK_INTERVAL);
  digitalWrite(LED, LOW);
  delay(BLINK_INTERVAL);
}
