/*
  Basic Communication Example

  this is the easiest example to try out with the Arduino ADK board
  using the ADB communication scenario. It sends data from an analog
  sensor plugged on pin A0 to the phone and reads touches from the
  phone screen into the Arduino board execution analogWrite commands 
  to pin D11

  by Mads Hobye and Benjamin Weber from Illutron.dk and bw.nu
 */

#include <SPI.h>

#include <Adb.h>
#define MAX_RESET 8



// Adb connection.
Connection * connection;

// Elapsed time for ADC sampling
long lastTime;
boolean r = true;

// Event handler for the shell connection. 
void adbEventHandler(Connection * connection, adb_eventType event, uint16_t length, uint8_t * data)
{
  int i;

  // Data packets contain two bytes, one for each servo, in the range of [0..180]
  if (event == ADB_CONNECTION_RECEIVE)
  {
   r = !r;
    analogWrite(11,data[0]);
    Serial.println("sd");
  }
 

}

void setup()
{
  pinMode(11, OUTPUT);

  // Initialise serial port
  Serial.begin(57600);
  
  // Note start time
  lastTime = millis();


  // Initialise the ADB subsystem.  
  ADB::init();

  // Open an ADB stream to the phone's shell. Auto-reconnect
  connection = ADB::addConnection("tcp:4567", true, adbEventHandler);  
}

void loop()
{  
  if ((millis() - lastTime) > 20)
  {
    uint16_t data = analogRead(A0);
    connection->write(2, (uint8_t*)&data);
    lastTime = millis();
  }

  // Poll the ADB subsystem.
  ADB::poll();
}

