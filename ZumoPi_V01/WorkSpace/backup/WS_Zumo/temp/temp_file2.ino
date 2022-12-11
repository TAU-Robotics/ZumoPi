#include <ArduinoJson.h> // IDE
#include <Wire.h>       //IDE
#include <Zumo32U4.h>    // IDE
#include "UARTReader.h" 

// Definitions and declarations

#define NUM_LINE_SENSORS 5
#define NUM_PROX_SENSORS 4
#define NUM_IMU_SENSORS 2
#define MAX_NUM_SENSORS 5
 
Zumo32U4ProximitySensors prox;
Zumo32U4LineSensors lineSensors;
Zumo32U4Buzzer buzzer;
Zumo32U4IMU imu;
Zumo32U4Motors motors;

// Serial coomunication
UARTReader uartReader(Serial, (char*)"json:", '\n',512); // Input source, header footer, max buffer size = default 512. 

// Allocate the JSON document
StaticJsonDocument<512> doc;
bool jsonReady = false;

String inputString;
unsigned int lineSensorValues[NUM_LINE_SENSORS];
unsigned int proxSensorValues[NUM_PROX_SENSORS];
int16_t imuSensorValues[NUM_IMU_SENSORS];
float joyX;
float joyY;
bool calibration_flag = true;
const uint16_t maxSpeed = 200;
int16_t lastError = 0;
int rightMotor = 0;
int leftMotor = 0;
bool stringComplete;
int16_t position = 0;
bool fork_flag = 0;

//functions



// Operates the same as read calibrated, but also returns an
// estimated position of the robot with respect to a line. The
// estimate is made using a weighted average of the sensor indices
// multiplied by 1000, so that a return value of 0 indicates that
// the line is directly below sensor 0, a return value of 1000
// indicates that the line is directly below sensor 1, 2000
// indicates that it's below sensor 2000, etc.  Intermediate
// values indicate that the line is between two sensors.  The
// formula is:
//
//    0*value0 + 1000*value1 + 2000*value2 + ...
//   --------------------------------------------
//         value0  +  value1  +  value2 + ...
//
// By default, this function assumes a dark line (high values)
// surrounded by white (low values).  If your line is light on
// black, set the optional second argument white_line to true.  In
// this case, each sensor value will be replaced by (1000-value)
// before the averaging.
int getLinePosition(unsigned int *sensor_values,
    unsigned int direction, unsigned int position, bool calibration_flag)
{
    unsigned char i, on_line = 0;
    unsigned long avg, avg1, avg2; // this is for the weighted total, which is long before division
    unsigned int sum, sum1, sum2; // this is for the denominator which is <= 64000
    avg = 0;
    avg1 = 0;
    avg2 = 0;
    sum = 0;
    sum1 = 0;
    sum2 = 0;

    for(i=0;i<NUM_LINE_SENSORS;i++) {
        int value = sensor_values[i];

        // keep track of whether we see the line at all
        if(value > 400) {
            on_line = 1;
        }

        // only average in values that are above a noise threshold
        if(value > 50) {
            avg += (long)(value) * (i * 1000);
            sum += value;
            if (i < 2)
            {
              avg1 += (long)(value) * (i * 1000);
              sum1 += value;
            }
            if (i > 2)
            {
              avg2 += (long)(value) * (i * 1000);
              sum2 += value;
            }
        }
    }

    if(!on_line)
    {
        // If it last read to the left of center, return 0.
        if(position < (NUM_LINE_SENSORS-1)*1000/2)
            return 0;

        // If it last read to the right of center, return the max.
        else
            return (NUM_LINE_SENSORS-1)*1000;

    }

    // Fork in the road
    if ((sum1 > 400 && sum2 > 400))
    {
      fork_flag = 1;
      //if (position < (NUM_LINE_SENSORS-1)*1000/2)
      if (direction == 0)
      {
        position = avg1/sum1;
      }
      else
      {
        position = avg2/sum2;
      }     
    }
    else
    {
      fork_flag = 0;
      position = avg/sum;
    }
    return position;
}

// serialEvent, This routine is run between each time loop() 
void mySerialEvent() { 
  while (Serial.available()) {
    // Get message:
    int ret_code = uartReader.getMessage();
    // if message complete Parse message:
    if(ret_code == uartReader.UART_GOT_MESSAGE) {
       //Serial.print(uartReader.buffer());
      deserializeJson(doc, uartReader.buffer());
      jsonReady = true;
    }else if(ret_code == uartReader.UART_BUFF_OVERFLOW){ 
      Serial.println("BUFF_OVERFLOW");
    }
  }
}

void calibrateSensors()
{

  // Wait 1 second and then begin automatic sensor calibration
  // by rotating in place to sweep the sensors over the line
  delay(1000);
  for(uint16_t i = 0; i < 120; i++)
  {
    if (i > 30 && i <= 90)
    {
      motors.setSpeeds(-200, 200);
    }
    else
    {
      motors.setSpeeds(200, -200);
    }

    lineSensors.calibrate();
  }
  motors.setSpeeds(0, 0);
}

void getImuSensorValues() {
  imu.read();
  imuSensorValues[0] = imu.a.x;
  imuSensorValues[1] = imu.a.y;
}

void getLineSensorValues(){
  lineSensors.readCalibrated(lineSensorValues);
}

void getProxSensorValues(){
  prox.read();
  proxSensorValues[0] = prox.countsLeftWithLeftLeds();
  proxSensorValues[1] = prox.countsFrontWithLeftLeds();
  proxSensorValues[2] = prox.countsFrontWithRightLeds();
  proxSensorValues[3] = prox.countsRightWithRightLeds();
}

// main functions

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  imu.init();
  imu.enableDefault();
  prox.initThreeSensors();
  delay(1000);
  lineSensors.initFiveSensors();
  delay(1000);  
  Serial.begin(115200);
  inputString.reserve(200);
}

void loop() {
  mySerialEvent();
  if (jsonReady == true){
    jsonReady = false;
    int timestamp = millis();
    //int ind0 = inputString.indexOf(';');  // find location of first delimiter ';'
    //String str = inputString.substring(0, ind0);   // capture first data String - auto_mode flag 
    int auto_mode = doc["auto_flag"];
    //int ind2 = inputString.indexOf('.'); // find location of second delimiter '.'
    //str = inputString.substring(ind0 + 1, ind2);   // capture second data String - direction
    int direction = doc["direction"];
    // Automatic
    if(auto_mode == 1){
        if(calibration_flag){
          calibrateSensors();
        }
      getLineSensorValues();
      position = getLinePosition(lineSensorValues, direction, position, calibration_flag);
      int16_t error = position - 2000;
      int16_t speedDifference = 0;
      if (fork_flag == 0)
      {
        speedDifference = error / 4 + 6 * (error - lastError);
      }
      else
      {
        speedDifference = error + 20 * (error - lastError);
      }
      lastError = error;
      int16_t leftSpeed = (int16_t)maxSpeed + speedDifference;
      int16_t rightSpeed = (int16_t)maxSpeed - speedDifference;
      leftSpeed = constrain(leftSpeed, 0, (int16_t)maxSpeed);
      rightSpeed = constrain(rightSpeed, 0, (int16_t)maxSpeed);
      motors.setSpeeds(leftSpeed, rightSpeed);
      calibration_flag = false;
    }

    // Manual
    else{
     // str = "";  
      //int ind1 = inputString.indexOf(',');  // find location of third delimiter ','
      //str = inputString.substring(ind2 + 1, ind1);   // capture third data String - joyX
      int joyX = doc["joyX"];
      //str = "";
      //str = inputString.substring(ind1+1);   // capture fourth data String - joyY
      int joyY = doc["joyY"];
      leftMotor = joyY + int(float(joyX)/1.5);
      rightMotor = joyY - int(float(joyX)/1.5);
      motors.setLeftSpeed(leftMotor);
      motors.setRightSpeed(rightMotor);
      calibration_flag = true;
    }
      
    // Assign sensor values into sensor arrays
    getImuSensorValues();
    getLineSensorValues();
    getProxSensorValues();

    // Read battery level
    uint16_t batteryLevel = readBatteryMillivolts();
    float battery = float(batteryLevel)/1000.0;

    // Send data to Pi by json document
    // data includes timestamp, battery and sensor values which are stored in arrays
    StaticJsonDocument<1024> jBuffer;
    JsonObject root = jBuffer.to<JsonObject>();
    JsonArray line_sensor_values = root.createNestedArray("line_sensor_values");
    JsonArray imu_sensor_values = root.createNestedArray("imu_sensor_values");
    JsonArray prox_sensor_values = root.createNestedArray("prox_sensor_values");
    root["timestamp"] = timestamp;
    root["Battery"] = battery;
    // fill each key's values according to the sensors values
    for (int i = 0; i < MAX_NUM_SENSORS; i++){
      if( i < NUM_IMU_SENSORS){
        imu_sensor_values.add(imuSensorValues[i]);
      }
      if (i < NUM_LINE_SENSORS){
        line_sensor_values.add(lineSensorValues[i]);
      }
      if(i < NUM_PROX_SENSORS){
      prox_sensor_values.add(proxSensorValues[i]);
      }
    }
    // document transmission
    String output_string;
    serializeJsonPretty(root, Serial);
    Serial.println();
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
}