//OFS libs
//#include <FastSerial.h>        //Removed this crap from all of the ArduPilot libs
#include <AP_Common.h>
#include <AP_Math.h>		// ArduPilot Mega Vector/Matrix math Library
#include <SPI.h>      		// Arduino SPI library
#include <AP_OpticalFlow.h> // ArduCopter OpticalFlow Library

//#include <Wire.h>

AP_OpticalFlow_ADNS3080 flowSensor(A3);  // override chip select pin to use A3 if using APM2

void setup() {
  // initialize serial ports:
  Serial.begin(115200);  // USB Console
  Serial.flush();
  
  //Wire.begin(); // join i2c bus (address optional for master)
  
  init_ofs();
  
}


void loop() {
  get_ofs();
}

void get_ofs(){
  flowSensor.update();
  flowSensor.update_position(0,0,0,1,100);
  
  String OFSString = "";

  OFSString.reserve(42); // reserve 42 bytes for OFSString
  
  OFSString = "$PRSO200,";
  
  OFSString += flowSensor.x,DEC;
  
  OFSString += ",";
  
  OFSString += flowSensor.dx,DEC;

  OFSString += ",";
  
  OFSString += flowSensor.y,DEC;

  OFSString += ",";
  
  OFSString += flowSensor.dy,DEC;

  OFSString += ",";

  OFSString += flowSensor.surface_quality,DEC;
  
  OFSString += "\r\n";
  
  Serial.print(OFSString);
/*
  char charBuf[42];
  OFSString.toCharArray(charBuf, 42);

  Wire.beginTransmission(4); // transmit to device Arduino Mega ADK
  Wire.write(charBuf);              // Send OFSString
  Wire.endTransmission();    // stop transmitting
*/

/*
  Serial.print("x/dx: ");
  Serial.print(flowSensor.x,DEC);
  Serial.print("/");
  Serial.print(flowSensor.dx,DEC);
  Serial.print("\ty/dy: ");
  Serial.print(flowSensor.y,DEC);
  Serial.print("/");
  Serial.print(flowSensor.dy,DEC);
  Serial.print("\tsqual:");
  Serial.print(flowSensor.surface_quality,DEC);
  Serial.println();
*/
  
}


void init_ofs(){
  // flowSensor initialization
  if( flowSensor.init() == false ) {
    Serial.println("Failed to initialise ADNS3080");
  }
  flowSensor.set_orientation(AP_OPTICALFLOW_ADNS3080_PINS_FORWARD);
  flowSensor.set_field_of_view(AP_OPTICALFLOW_ADNS3080_08_FOV);
  
  flowSensor.clear_motion(); //zero everything
}
