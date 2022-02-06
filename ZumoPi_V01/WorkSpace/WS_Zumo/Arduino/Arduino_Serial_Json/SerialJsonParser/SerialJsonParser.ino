// example for serial stream handler with json parser library

// example message:
// "json:{\"status\":\"on\",\"time\":100,\"data\":[10,20]}";
// "json:{\"time\":100}"

// libraries
#include <ArduinoJson.h>
#include "UARTReader.h" 

// Serial coomunication
UARTReader uartReader(Serial, (char*)"json:", '\n',512); // Input source, header footer, max buffer size = default 512. 

// Allocate the JSON document
StaticJsonDocument<512> doc;
bool jsonReady = false;

// init
void setup() {
  // init serial
  Serial.begin(115200);
}

// main loop
void loop() {
  mySerialEvent();
  if (jsonReady == true){
    jsonReady = false;
    // get various elemets from the json object and print them
    long timestamp = doc["time"];
    int data = doc["data"][0];
    
    Serial.print("time: ");
    Serial.println(timestamp);
    Serial.print("data: ");
    Serial.println(data);
    
    // update elements
    doc["time"] = millis();;
    doc["data"][0] = 100;
    
    // print JSON content
    String output;
    serializeJson(doc, output);
    Serial.println(output);
  }
}

// serialEvent, This routine is run between each time loop() 
void mySerialEvent() { 
  while (Serial.available()) {
    // Get message:
    int ret_code = uartReader.getMessage();
    // if message complete Parse message:
    if(ret_code == uartReader.UART_GOT_MESSAGE) {
      Serial.print(uartReader.buffer());
      deserializeJson(doc, uartReader.buffer());
      jsonReady = true;
    }else if(ret_code == uartReader.UART_BUFF_OVERFLOW){ 
      Serial.println("BUFF_OVERFLOW");
    }
  }
}
