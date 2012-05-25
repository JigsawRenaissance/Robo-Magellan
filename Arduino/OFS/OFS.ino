//OFS libs
//#include <FastSerial.h>        //Removed this crap from all of the ArduPilot libs
#include <AP_Common.h>
#include <AP_Math.h>		// ArduPilot Mega Vector/Matrix math Library
#include <SPI.h>      		// Arduino SPI library
#include <AP_OpticalFlow.h> // ArduCopter OpticalFlow Library

#define BUFFSIZ 42

AP_OpticalFlow_ADNS3080 flowSensor(A3);  // override chip select pin to use A3 if using APM2

void setup() {
  // initialize serial ports:
  Serial.begin(19200);  // USB Console
  Serial.flush();
  
  init_ofs();
  
}


void loop() {
  get_ofs();
}

void get_ofs(){
  flowSensor.update();
  flowSensor.update_position(0,0,0,1,100);
  
  String OFSString = "";

  OFSString.reserve(BUFFSIZ); // reserve 42 bytes for OFSString
  
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
  
  OFSString += "*";
   
  char charBuf[BUFFSIZ];
  OFSString.toCharArray(charBuf, BUFFSIZ);

  char chksum[2];
  if(checksum(chksum, charBuf)) {

    OFSString += chksum[0];
    OFSString += chksum[1];
  
    OFSString += "\r\n";
  
    Serial.print(OFSString);
  }
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


bool checksum(char* chksum, char* msg)
// Assume that message starts with $ and ends with *
{
    int sum = 0;
    if(msg[0] != '$')
      return false;
    for (int i = 1; i < BUFFSIZ-5; i++)
    {
      if(msg[i] == '*')
      {
         int msb = (sum>>4);
         chksum[0] = msb>9? 'A'+msb-10 : '0'+msb;
         int lsb = (sum&0x0F);
         chksum[1] = lsb>9? 'A'+lsb-10 : '0'+lsb;
         return(true);
      }
      sum ^= msg[i];
    }
    return(false);
}
