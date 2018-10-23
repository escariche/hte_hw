/*
  433 MHz RF Module Transmitter Demonstration 1
  RF-Xmit-Demo-1.ino
  Demonstrates 433 MHz RF Transmitter Module
  Use with Receiver Demonstration 1

  DroneBot Workshop 2018
  https://dronebotworkshop.com
*/

// Include RadioHead Amplitude Shift Keying Library
#include <RH_ASK.h>
// Include dependant SPI Library 
#include <SPI.h> 
// Include DHT library
#include "DHT.h"
// Include json library
#include <ArduinoJson.h>

#define MYID "01"      // the ID number of this board (tens=0 means RX, unit is id). Change this for each board.
#define SLEEPTIME 1000
#define DHTPIN 11     // what pin the DHT is connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

DHT dht(DHTPIN, DHTTYPE);

// Create Amplitude Shift Keying Object
RH_ASK rf_driver;

void setup(){
  Serial.begin(9600);   // Debugging only

  // Initialize ASK Object
  rf_driver.init();
  
  //Initialize the Sensor
  dht.begin();
  if (!rf_driver.init()){
      Serial.println("init failed");
    }
}

int ftoa(char *a, float f){  //translates floating point readings into strings to send over the air
  int left=int(f);
  float decimal = f-left;
  int right = decimal *100; //2 decimal points
  if (right > 10) {  //if the decimal has two places already. Otherwise
    sprintf(a, "%d.%d",left,right);
  } else { 
    sprintf(a, "%d.0%d",left,right); //pad with a leading 0
  }
}

int xmitMessage(char *msg){
    //const char *msg = "Welcome to the Workshop!";
    rf_driver.send((uint8_t *)msg, strlen(msg));
    rf_driver.waitPacketSent();
    delay(SLEEPTIME);
}

JsonObject& buildJSON(float temp, float hum){
  // Memory pool for JSON object tree.
  //
  // Inside the brackets, 200 is the size of the pool in bytes.
  // Don't forget to change this value to match your JSON document.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonBuffer<40> jsonBuffer;
  
  // Create the root of the object tree.
  //
  // It's a reference to the JsonObject, the actual bytes are inside the
  // JsonBuffer with all the other nodes of the object tree.
  // Memory is freed when jsonBuffer goes out of scope.
  JsonObject& root = jsonBuffer.createObject();
  // Add values in the object
  //
  // Most of the time, you can rely on the implicit casts.
  // In other case, you can do root.set<long>("time", 1351824120);
  root["s"] = MYID;
  // Add values in the object
  //root["time"] = millis();
  
   // Add a nested array.
  //
  // It's also possible to create the array separately and add it to the
  // JsonObject but it's less efficient.
  root["tp"] = temp;
  root["hm"] = hum;
  //Serial.print("In buildjson");
  //root.printTo(Serial);
  return root;
}


void loop(){
  char message[50];
  float t = dht.readTemperature();
  float h = dht.readHumidity();
    
  //build the message
  char temp[6]; //2 int, 2 dec, 1 point, and \0
  ftoa(temp,t);
  char hum[6]; //2 int, 2 dec, 1 point, and \0
  ftoa(hum,h);
  
  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(t)||isnan(h)) {
    sprintf(message, "ID:%d:TS:%lu:ER:ERROR\0", MYID, millis());  //millis provides a stamp for deduping if signal is repeated
    Serial.println("Failed to read from DHT");
    xmitMessage(message);
  } else {
    JsonObject& jsonData = buildJSON(t, h);
    jsonData.printTo(Serial);
    Serial.println();
    char jsonChar[40];
    jsonData.printTo((char*)jsonChar, jsonData.measureLength() + 1);
    xmitMessage(jsonChar);  //message will not be sent if there is an error
    delay(SLEEPTIME);  //Sleep randomly to avoid cross talk with another unit
  }
  
}
