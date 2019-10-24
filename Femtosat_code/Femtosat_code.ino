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

//Notes
//IMU - b110100X(RW) - 0xD1

//BMU - b111011X(RW) - 0xED
//Registers 0xF6, 0xF7


//include libraries
#include <SPI.h>
#include <RFM69.h>
#include <MPU9250.h>
#include <SparkFunBME280.h>

//Define Radio
#define NETWORKID     0   // Must be the same for all nodes
#define MYNODEID      1   // My node ID
#define TONODEID      2   // Destination node ID
#define FREQUENCY     RF69_915MHZ

//Define IMU
#define I2Cclock 400000
#define I2Cport Wire
#define MPU9250_ADDRESS MPU9250_ADDRESS_AD0   // Use either this line or the next to select which I2C address your device is using
//#define MPU9250_ADDRESS MPU9250_ADDRESS_AD1

//Define Objects
RFM69 radio;
MPU9250 myIMU(MPU9250_ADDRESS, I2Cport, I2Cclock);
BME280 myBME; //Uses default I2C address 0x77

//Variables
float sendBuffer[62];
int bufferIndex = 0;

void setup() {

  Wire.begin();
  Serial.begin(38400);  //Initialize Serial Communication
  
  //Wait for Serial connection
  while(!Serial){};

  //Init radio communication
  radio.initialize(FREQUENCY, MYNODEID, NETWORKID);
  radio.setHighPower();
  radio.setPowerLevel(20);
  radio.encrypt(0);

  //Init the IMU
  doIMUSelfTest();

  //Init the BME
  myBME.setI2CAddress(0x77);
  if(myBME.beginI2C() == false) Serial.println("Sensor A connect failed");


}

void loop() {
  
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

  myIMU.readMagData(myIMU.magCount);  // Read the x/y/z adc values

  // Calculate the magnetometer values in milliGauss
  // Include factory calibration per data sheet and user environmental
  // corrections
  // Get actual magnetometer value, this depends on scale being set
  myIMU.mx = (float)myIMU.magCount[0] * myIMU.mRes
             * myIMU.factoryMagCalibration[0] - myIMU.magBias[0];
  myIMU.my = (float)myIMU.magCount[1] * myIMU.mRes
             * myIMU.factoryMagCalibration[1] - myIMU.magBias[1];
  myIMU.mz = (float)myIMU.magCount[2] * myIMU.mRes
             * myIMU.factoryMagCalibration[2] - myIMU.magBias[2];

  float imuData[] = {myIMU.ax, myIMU.ay, myIMU.az, myIMU.gx,
     myIMU.gy, myIMU.gz, myIMU.mx, myIMU.my, myIMU.mz};
     
  for (int i = 0; i < 9; i++){
    sendBuffer[bufferIndex] = imuData[i];
    bufferIndex++;
  }
  
  //Fetch and put BME data in buffer
  float BMEData[] = {myBME.readFloatHumidity(),myBME.readFloatPressure(),
    myBME.readTempF()};
  for (int i = 0; i < sizeof(BMEData); i++){
    sendBuffer[bufferIndex] = BMEData[i];
    bufferIndex++;
  }

   
  //If enough data saved in buffer
  //Send data packet by radio
  //and write to memory module
  
  if (bufferIndex >= 54) {
    PrintBuffer(sendBuffer, bufferIndex);
    Serial.println("Sending by Radio...");
    radio.send(TONODEID, sendBuffer, min(bufferIndex,61), false);
    bufferIndex = 0;
    Serial.println("Transmission complete!");
  }

  delay(100);

}

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

void PrintBuffer(float sendbuf[], int index){
  Serial.print("[");
  for(int i = 0; i < index; i++){
    Serial.print(sendbuf[i], HEX);
    Serial.print(", ");
  }
  Serial.println("]");
  return;
}
