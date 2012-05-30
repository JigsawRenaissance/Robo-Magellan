/*

http://www.circuitsathome.com/mcu/google-open-accessory-interface-for-usb-host-shield-library-2-0-released

*/

#include <SPI.h>
#include <Adb.h>

#include <Servo.h>

#define PMTK_API_SET_NMEA_OUTPUT "$PMTK314,5,5,5,5,5,5,5,5,5,5,5,5,0,5,5,5,5*28\r\n"
//#define PMTK_API_SET_NMEA_OUTPUT "$PMTK314,-1*04\r\n"  // Default settings

#define PWR_SAV 0
#define PWR_SAV_OFF "$PMTK320,0*2F\r\n"
#define PWR_SAV_ON "$PMTK320,1*2E\r\n"

#define USE_SBAS 1
#define SBAS_ON "$PMTK313,1*2E\r\n"
#define SBAS_OFF "$PMTK313,0*2F\r\n"

#define USE_WAAS 1 //1 = Enable, 0 = Disable, good in USA, slower FIX...
#define WAAS_ON "$PMTK301,2*2E\r\n" // enable WAAS
#define WAAS_OFF "$PMTK301,1*2D\r\n" // disable WAAS

#define  SERVO0         10 //Throttle
#define  SERVO1         11 //Steering
#define  SERVO2         12 //Tilt
#define  SERVO3         13 //Pan

#define MIN_STEERING_LIMIT 86
#define MAX_STEERING_LIMIT 200

#define SONARBUF 18

#define SONAR1 A0
#define SONAR2 A1
#define SONAR3 A2
#define SONAR4 A3
#define SONAR5 A4

char sonar[5] = {SONAR1, SONAR2, SONAR3, SONAR4, SONAR5};

uint8_t rcv_buffer[3];
int rcv_buffer_offset = 0;

int sonar_val[5];

#define SONAR_TRIGGER 6  //pin 6 is the trigger

// Adb connection.
Connection * connection;

char ConsoleString[1024];         // a string to hold incoming console data
int ConsoleIndex=0;
boolean ConsoleComplete = false;   // This will be set to true once we have a full string

Servo servo[4];

String GPSString = "";         // a string to hold incoming GPS data
boolean GPSComplete = false;   // This will be set to true once we have a full string

String LRFString = "";         // a string to hold incoming LRF data
boolean LRFComplete = false;   // This will be set to true once we have a full string

String OFSString = "";         // a string to hold incoming OFS data
boolean OFSComplete = false;   // This will be set to true once we have a full string

String SonarString = "";

void setup() {
    
  // initialize serial ports:
  Serial.begin(115200);  // USB Console
  Serial.flush();

  SonarString.reserve(SONARBUF);

  init_servo();
  
  init_lrf();
  
  init_gps();
  
  init_ofs();
  
  init_sonar();
  
  // Initialise the ADB subsystem.  
  ADB::init();
  
  // Open an ADB stream to the phone's shell. Auto-reconnect
  //connection = ADB::addConnection("shell:exec logcat", true, adbEventHandler);
  //connection = ADB::addConnection("shell:exec logcat -s Cyclops:D", true, adbEventHandler);
  //connection = ADB::addConnection("shell:exec sh", true, adbEventHandler);  
  connection = ADB::addConnection("tcp:1137", true, adbEventHandler);

}

void loop() {
  
  // Poll the ADB subsystem.
  ADB::poll();
  
  if (GPSComplete) {
    Serial.print(GPSString);
    char charBuf[82];
    GPSString.toCharArray(charBuf, 82);
    connection->writeString(charBuf);
    // clear the string:
    GPSString = "";
    GPSComplete = false;
  }
  
  if (LRFComplete) {
    //Serial.println(LRFString);
    char charBuf[13];
    LRFString.toCharArray(charBuf, 13);
    connection->writeString(charBuf);
    // clear the string:
    LRFString = "";
    LRFComplete = false;
  }
   
  if (ConsoleComplete) {
   connection->write(ConsoleIndex, (uint8_t*)ConsoleString);
   ConsoleComplete = false;
   ConsoleIndex=0;
  }
  
  if (OFSComplete) {
    Serial.print(OFSString);
    char charBuf[42];
    OFSString.toCharArray(charBuf, 42);
    connection->writeString(charBuf);
    // clear the string:
    OFSString = "";
    OFSComplete = false;
  }
  
  
  read_sonar();
  
  //get_lrf_range();
  
  
  /* Drive in a circle */
  /*
  servo[3].write(0); //Pan

  servo[3].write(180); //Pan
  
  servo[0].write(110); //Throttle

  servo[1].write(map(200, 0, 255, 0, 180)); //Steering
 	*/

  //pan();
}

// Event handler for the adb connection. 
void adbEventHandler(Connection * connection, adb_eventType event, uint16_t length, uint8_t * data)
{

  if (event == ADB_CONNECTION_RECEIVE) {
  	
  	//Serial.print("length = ");
  	//Serial.println(length);
  	for (int i=0; i<length; i++) {
      //Serial.println(data[i]);
      //Serial.write(data[i]);

			rcv_buffer[rcv_buffer_offset]=data[i];
  		rcv_buffer_offset++;
  		if (rcv_buffer_offset == 3 ) {
  			processADBMessage();
  		}
    }  	
  }
  
  if (event == ADB_CONNECTION_FAILED) {
    Serial.println("ADB_CONNECTION_FAILED");
  }

  if (event == ADB_CONNECTION_OPEN) {
    Serial.println("ADB_CONNECTION_OPEN");
  }

  if (event == ADB_CONNECTION_CLOSE) {
    Serial.println("ADB_CONNECTION_CLOSE");
  }


  if (event == ADB_CONNECT) {
    Serial.println("ADB_CONNECT");
  }

  if (event == ADB_DISCONNECT) {
    Serial.println("ADB_DISCONNECT");
  }

}

void serialEvent(){ //Interrupt when the Console says something
  while (Serial.available() && !ConsoleComplete) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to ConsoleString:
    ConsoleString[ConsoleIndex] = inChar;
    ConsoleIndex++;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      ConsoleComplete = true;
    } 
  }
}

void serialEvent1(){ //Interrupt when the LRF says something
  while (Serial1.available() && !LRFComplete) {
    // get the new byte:
    char inChar = (char)Serial1.read();

    /*    
    Serial.print("LRFString=\"");
    Serial.print(LRFString);
    Serial.println('"');
    */
    
    //Serial.print(inChar);
    // add it to LRFString:
    LRFString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:

    //Serial.print("Before LRFString = ");
    //Serial.println(LRFString);

    
    if (LRFString.substring(0,1) != "D") {
      LRFString="";
    } else {
      if (LRFString.substring(0,4) == "D = " && LRFString.substring(8,11) == " mm") {
        //Serial.print("Foo LRFString = ");
        //Serial.println(LRFString.substring(4,8));
        LRFString += "\n";
        LRFComplete = true;
      }
    }

    //Serial.print("After LRFString = ");
    //Serial.println(LRFString);

  }
}

void serialEvent2(){ //Interrupt when the GPS says something
  while (Serial2.available() && !GPSComplete) {
    // get the new byte:
    char inChar = (char)Serial2.read();
    // add it to GPSString:
    GPSString += inChar;    
    /*
    if (GPSString.substring(0,1) != "$") { //If it doesn't start with a $ then it isn't valid.
      GPSString="";
    }
    */
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      GPSComplete = true;
    } 
  }
}

void serialEvent3(){ //Interrupt when the OFS says something
  while (Serial3.available() && !OFSComplete) {
    // get the new byte:
    char inChar = (char)Serial3.read();
    // add it to GPSString:
    OFSString += inChar;    
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      OFSComplete = true;
    } 
  }
}


void init_lrf() {
  Serial1.begin(19200); // LRF
  LRFString.reserve(13); // reserve 13 bytes for LRFString

  Serial.print("Initializing LRF");

  Serial1.println("");
  Serial1.println("");
  Serial1.println("");

  //Serial1.print("V");

  delay(100);
  
  Serial1.print("S");

  delay(100);
  
  Serial1.print("E");

  for (int i = 1; i <= 10; i++)
  {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println(" Done!");
}

void init_gps() {
  Serial2.begin(57600);  // GPS
  GPSString.reserve(82);   // reserve 82 bytes for GPSString. This is the longest possibe NMEA sentence

  #if USE_WAAS ==1
  Serial2.print(WAAS_ON);
  #else
  Serial2.print(WAAS_OFF);
  #endif

  #if USE_SBAS ==1
  Serial2.print(SBAS_ON);
  #else
  Serial2.print(SBAS_OFF);
  #endif

  #if PWR_SAV ==1
  Serial2.print(PWR_SAV_ON);
  #else
  Serial2.print(PWR_SAV_OFF);
  #endif

  Serial2.print(PMTK_API_SET_NMEA_OUTPUT);

}

void init_servo() {
        servo[0].attach(SERVO0);
        servo[1].attach(SERVO1);
        servo[2].attach(SERVO2);
        servo[3].attach(SERVO3);

        
        servo[0].write(90); //Throttle
        servo[1].write(90); //Steering
        servo[2].write(0); //Tilt
        servo[3].write(100); //Pan

        
}


void init_ofs() {
  Serial3.begin(19200);  // OFS
  OFSString.reserve(42);   // reserve 42 bytes for OFSString. This is the longest possibe NMEA sentence

}

void get_lrf_range(){
  Serial1.print("R");
}


void pan(){
  servo[3].write(0); //Pan

  delay(1000);

  servo[3].write(100); //Pan

  delay(1000);
  
  servo[3].write(180); //Pan

  delay(1000);

  servo[3].write(100); //Pan

  delay(1000);
}

void init_sonar(){
  pinMode(SONAR_TRIGGER, OUTPUT);
  digitalWrite(SONAR_TRIGGER, LOW);
  delay(2000);

  
  //delay(250); // Wait at least 250ms for the sonars to boot up
  digitalWrite(SONAR_TRIGGER, HIGH);
  delayMicroseconds(40); // Wait at least 20us
  digitalWrite(SONAR_TRIGGER, LOW);
  pinMode(SONAR_TRIGGER, INPUT); // electrically disconnects the pin
  delay(100); // Wait 100mS if this is the first time reading the sensor as it calibrates upon the first commanded range cycle after power up
}

void read_sonar(){
  for (int i=0; i < 5; i++) {
    sonar_val[i] = analogRead(sonar[i]);
    
    SonarString = "$PRSO30";
    SonarString += i;
    SonarString += ",";
    SonarString += sonar_val[i];
    SonarString += "*";

    char charBuf[SONARBUF];
    SonarString.toCharArray(charBuf, SONARBUF);

    char chksum[2];
    if(checksum(chksum, charBuf)) {

      SonarString += chksum[0];
      SonarString += chksum[1];
  
      SonarString += "\r\n";
  
      SonarString.toCharArray(charBuf, SONARBUF);
      connection->writeString(charBuf);
  
      Serial.print(SonarString);
    }
  }
}


bool checksum(char* chksum, char* msg)
// Assume that message starts with $ and ends with *
{
    int sum = 0;
    if(msg[0] != '$')
      return false;
    for (int i = 1; i < SONARBUF-5; i++)
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



void processADBMessage(){

  rcv_buffer_offset=0;

	if (rcv_buffer[0] == 0x2) {
	 switch( rcv_buffer[1] ) {
	 	case 1:
	 		if(rcv_buffer[2] < MIN_STEERING_LIMIT) rcv_buffer[2] = MIN_STEERING_LIMIT;
	     if(rcv_buffer[2] > MAX_STEERING_LIMIT) rcv_buffer[2] = MAX_STEERING_LIMIT;
	   	Serial.print("Steering to ");
	   	Serial.println(rcv_buffer[2]);
	   	servo[1].write(map(rcv_buffer[2], 0, 255, 0, 180)); //Steering
	    break;
	 	case 2:
	   	Serial.print("Throttle to ");
	   	Serial.println(rcv_buffer[2]);
	   	servo[0].write(map(rcv_buffer[2], 0, 255, 0, 180)); //Throttle
	    break;
	   case 3:
	   	Serial.print("Turret pan to ");
	   	Serial.println(rcv_buffer[2]);
	   	servo[3].write(map(rcv_buffer[2], 0, 255, 0, 180)); //Pan
	    break;
	   case 4:
	   	Serial.print("Turret tilt to ");
	   	Serial.println(rcv_buffer[2]);
	   	servo[2].write(map(rcv_buffer[2], 0, 255, 0, 180)); //Tilt
	    break;
	   case 5:
	   	Serial.print("Getting LRF: ");
	   	Serial.println(rcv_buffer[2]);
	   	if (rcv_buffer[2] == 0x1) {
	   		get_lrf_range();
	   	}
	    break;
		}
	}
}