#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile
#include <ArduinoJson.h>
#include <DHT.h>

#define MYID "board_tx_00"      //the ID number of this board.  Change this for each board you flash.
#define TRANSPIN 12  //what pin to transmit on
#define ledPin 13 //Onboard LED = digital pin 13
#define DHTPIN 11 // pin for DHT sensor

#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

RH_ASK driver;

void setup(){
  Serial.begin(9600);   // Debugging only

   dht.begin();
   pinMode(ledPin,OUTPUT);
    if (!driver.init()){
      Serial.println("init failed");
    }
}

int ftoa(char *a, float f)  //translates floating point readings into strings to send over the air
{
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
  //const char *msg = "Hello World!";
    digitalWrite(LED_BUILTIN, true); // Flash an led to show transmitting
    driver.send((uint8_t *)msg, strlen(msg));
    driver.waitPacketSent();
    delay(1000);
    digitalWrite(LED_BUILTIN, false); // Flash an led to show transmitting
}

JsonObject& buildJSON(float temp, float humid){
  // Memory pool for JSON object tree.
  //
  // Inside the brackets, 200 is the size of the pool in bytes.
  // Don't forget to change this value to match your JSON document.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonBuffer<200> jsonBuffer;
  
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
  root["sensor"] = MYID;
  // Add values in the object
  root["time"] = millis();
  
   // Add a nested array.
  //
  // It's also possible to create the array separately and add it to the
  // JsonObject but it's less efficient.
  root["temperature"] = temp;
  root["humidity"] = humid;
  root.printTo(Serial);
  return root;
}


void loop(){
  char message[50];
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float tf = t * 1.8 +32;  //Convert from C to F*/ 

//build the message
  char temp[6]; //2 int, 2 dec, 1 point, and \0
  char hum[6];
  /*if (UNIT == 0 ){  //choose the right unit F or C
    ftoa(temp,tf);
  }
  else {
    ftoa(temp,t);
  }*/
  ftoa(temp,t);
  ftoa(hum,h);
  
  // check if returns are valid, if they are NaN (not a number) then something went wrong!
    if (isnan(t) || isnan(h)) {
      sprintf(message, "ID:%d:TS:%lu:ER:ERROR\0", MYID, millis());  //millis provides a stamp for deduping if signal is repeated
      Serial.println("Failed to read from DHT");
      xmitMessage(message);
    } else {
      JsonObject& jsonData = buildJSON(t, h);
      Serial.print("Humidity: "); 
      Serial.print(h);
      Serial.print(" %\t");
      Serial.print("Temperature: "); 
      Serial.print(t);
      Serial.println(" *C");
      Serial.print("Sending Message: ");
      sprintf(message, "ID:%d:TS:%lu:TC:%s:RH:%s\0", MYID, millis(), temp, hum);  //millis provides a stamp for deduping if signal is repeated
      jsonData.printTo(Serial);
      char jsonChar[100];
      jsonData.printTo((char*)jsonChar, jsonData.measureLength() + 1);
      xmitMessage(jsonChar);  //message will not be sent if there is an error
    }
    unsigned long randNumber = random(5,10); //1 to 2 minutes to delay
    unsigned long sleepTime=randNumber*1000;
    //Serial.print("Sleeping ");
    //Serial.print(sleepTime);
    //Serial.println(" miliseconds");
    delay(sleepTime);  //Sleep randomly to avoid cross talk with another unit
}
