/*
  Odometry with teleoperate
*/
#include <Wire.h>
#include <Zumo32U4.h>

// zumo classes
Zumo32U4Encoders encoders;
Zumo32U4Motors motors;


// message variables
String inputString = "";      // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

// Odometry settings
#define GEAR_RATIO 100.37      // Motor gear ratio
#define WHEELS_DISTANCE 98 // Distance between tracks
#define WHEEL_DIAMETER 37.5   // Wheels diameter measured 38.5
#define ENCODER_PPR 12        // Encoder pulses per revolution
float encoder2dist = WHEEL_DIAMETER*3.14/(ENCODER_PPR*GEAR_RATIO);  // conversition of encoder pulses to distance in mm

float theta = 0; 
float posx = 0;
float posy = 0;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  
  // initialize serial:
  Serial.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  // update motors command to stop
  motors.setLeftSpeed(0);
  motors.setRightSpeed(0);
}

// the loop function runs over and over again forever
void loop() {
  msg_handler();
  odometry();
  delay(1);
}



// check if there is a message if so parse it and send an update
void msg_handler(void){
  // check for incoming message
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
  // check if message is complete
  if (stringComplete) {
    parse_msg();
  }
}
// parse incoming message and send a response
void parse_msg(void){
    //Serial.print(inputString);

    int ind1 = inputString.indexOf(',');  //finds location of first ,
    String str = inputString.substring(0, ind1);   //captures first data String
    int joyX = str.toInt();
    str ="";
    str = inputString.substring(ind1+1);   //captures first data String
    int joyY = str.toInt();
    
    int leftMotor = joyY + joyX;  //int(float(joyX)/1.5);
    int rightMotor = joyY - joyX; //int(float(joyX)/1.5);
    uint16_t batteryLevel = readBatteryMillivolts();
    float battery = float(batteryLevel)/1000.0;


    // send a response
    Serial.print(leftMotor);
    Serial.print(" , ");
    Serial.print(rightMotor);
    Serial.print(" , ");
    Serial.print(battery);
    Serial.print(" , ");
    Serial.print(posx);
    Serial.print(" , ");
    Serial.print(posy);
    Serial.print(" , ");
    Serial.print(theta*57.295);
    Serial.println(" h1 ");

    // update motors    
    motors.setLeftSpeed(leftMotor);
    motors.setRightSpeed(rightMotor);
    // clear the string:
    inputString = "";
    stringComplete = false;  
}

void odometry(void){
      //encoder read
    int16_t countsLeft = encoders.getCountsAndResetLeft();
    int16_t countsRight = encoders.getCountsAndResetRight();
    float dx_1 = countsRight*encoder2dist;
    float dx_2 = countsLeft*encoder2dist;
    posx += sin(theta)*(dx_1+dx_2)/2;
    posy += cos(theta)*(dx_1+dx_2)/2;
    theta += float(dx_1-dx_2)/WHEELS_DISTANCE;
}