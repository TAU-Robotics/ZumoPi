#include <ArduinoJson.h> // By Benoit Blanchon
#include <Wire.h>        // Arduino Library
#include <Zumo32U4.h>    // By Pololu

// Definitions and declarations

#define NUM_LINE_SENSORS 5
#define NUM_PROX_SENSORS 4
#define NUM_IMU_SENSORS 2
#define MAX_NUM_SENSORS 5
#define MSG_TIMEOUT_MILLIS 5000
#define DT_CONTROL_MILLIS 10  // Control DT suggested range 5-50 mSec --> 200-20 Hz
#define ON_LINE_THRESHOLD 400
#define NOISE_THRESHOLD 50
 
Zumo32U4ProximitySensors prox;
Zumo32U4LineSensors lineSensors;
Zumo32U4Buzzer buzzer;
Zumo32U4IMU imu;
Zumo32U4Motors motors;

// sensors arrays

unsigned int lineSensorValues[NUM_LINE_SENSORS];
unsigned int proxSensorValues[NUM_PROX_SENSORS];
int16_t imuSensorValues[NUM_IMU_SENSORS];

// control & communication variables

float joyX;
float joyY;
bool calibration_flag = true;
const uint16_t maxSpeed = 200;
int16_t lastError = 0;
int rightMotor = 0;
int leftMotor = 0;
String inputString;
bool stringComplete;
int16_t position = 0;
bool reached_junction = false;
int direction = 0;

bool communication_timed_out = 0;
unsigned long msg_time = 0;
unsigned long millisTick = 0;
float dt = DT_CONTROL_MILLIS / 1000.0;
uint16_t Kp_low = 0.25;
uint16_t Kd_low = 6;
uint16_t Kp_high = 1;
uint16_t Kd_high = 20;
int ind2;
String str;
int auto_mode;

// Functions

/*
This function returns an estimated position of the robot with respect to a line. The estimate is made 
using a weighted average of the sensor indices multiplied by 1000, so that a return value of 0 indicates 
that the line is directly below sensor 0, a return value of 1000 indicates that the line is directly below 
sensor 1, 2000 indicates that it's below sensor 2000, etc. Intermediate values indicate that the line is 
between two sensors. The formula is:

  0*value0 + 1000*value1 + 2000*value2 + ...
 --------------------------------------------
       value0  +  value1  +  value2 + ...

By default, this function assumes a dark line (high values) surrounded by white (low values). 
If your line is light on black, set the optional second argument white_line to true. In this case, 
each sensor value will be replaced by (1000-value) before the averaging.
*/
int getLinePosition(unsigned int *sensor_values, unsigned int direction, 
                    unsigned int position, bool calibration_flag)
{
    unsigned char i;
    unsigned long avg, avg1, avg2; // numerators for the weighted total, they're long before division
    unsigned int sum, sum1, sum2; // denominators for the weighted total, they're <= 64000
    avg = 0; avg1 = 0; avg2 = 0;
    sum = 0; sum1 = 0; sum2 = 0;
    bool on_line = false;

    for (i = 0; i < NUM_LINE_SENSORS; i++) {
        int value = sensor_values[i];

        // keep track of whether we see the line at all
        if (value > ON_LINE_THRESHOLD) {
            on_line = true;
        }

        // only average in values that are above the noise threshold
        if (value > NOISE_THRESHOLD) 
        {
            // Main values
            avg += (long)(value) * (i * 1000);
            sum += value;
            
            // Leftside values - will be used in case of a junction
            if (i < 2)
            {
              avg1 += (long)(value) * (i * 1000);
              sum1 += value;
            }

            // Rightside values - will be used in case of a junction
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

    // Reached a junction
    if ((sum1 > ON_LINE_THRESHOLD && sum2 > ON_LINE_THRESHOLD))
    {
      reached_junction = true;
      
      // Driving direction is counterclockwise - calculate position based on leftside values
      if (direction == 0)
      {
        position = avg1 / sum1;
      }

      // Driving direction is clockwise - calculate position based on rightside values
      else
      {
        position = avg2 / sum2;
      }     
    }
    
    // No junction
    else
    {
      reached_junction = false;
      position = avg / sum;
    }
    
    return position;
}

/*This function calibrates the line sensors. It is called every time the driving mode changes from 
manual to automatic.*/
void calibrateSensors()
{
  // Wait 1 second before beggining the automatic sensor calibration
  delay(1000);

  // Calibration is done by rotating in place to sweep the sensors over the line
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

// Read the inertial sensors (IMU)
void getImuSensorValues() {
  imu.read();
  imuSensorValues[0] = imu.a.x;
  imuSensorValues[1] = imu.a.y;
}

// Read the line sensors
void getLineSensorValues(){
  lineSensors.readCalibrated(lineSensorValues);
}

// Read the proximity sensors
void getProxSensorValues(){
  prox.read();
  proxSensorValues[0] = prox.countsLeftWithLeftLeds();
  proxSensorValues[1] = prox.countsFrontWithLeftLeds();
  proxSensorValues[2] = prox.countsFrontWithRightLeds();
  proxSensorValues[3] = prox.countsRightWithRightLeds();
}

// setup & loop functions

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  imu.init();
  imu.enableDefault();
  prox.initThreeSensors();
  delay(1000);
  lineSensors.initFiveSensors();
  delay(1000);
  Serial.begin(9600);
  inputString.reserve(200);
}

void loop() {
  
  // There are bytes available for reading at the serial port
  while (Serial.available()) {
    
    // Read the serial receive byffer one byte at a time until '\n'
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    }
  }

  // The message is ready to be parsed
  if (stringComplete) {
    
    // Get timestamp
    int timestamp = millis();
    
    // Read first parameter - auto_mode: manual (0) or automatic (1)
    int ind1 = inputString.indexOf(';');  // find location of first delimiter ';'
    str = inputString.substring(0, ind1);   // capture the auto_mode substring
    auto_mode = str.toInt(); // convert auto_mode to integer
    
    // Read the second parameter - direction: counterclockwise (0) or clockwise (1)
    ind2 = inputString.indexOf('.'); // find location of second delimiter '.'
    str = inputString.substring(ind1 + 1, ind2);   // capture the direction substring
    int direction = str.toInt(); // convert direction to integer
    
    // Automatic mode
    if(auto_mode == 1){
        
        // Calibrate line sensors. Happens only once when starting automatic mode.
        if(calibration_flag){
          calibrateSensors();
        }
      
      // Read line sensor values and populate the lineSensorValues array
      getLineSensorValues();
      
      // Calculate position of robot relative to the line, and the error
      position = getLinePosition(lineSensorValues, direction, position, calibration_flag);
      int16_t error = position - 2000;
      
      /*Implemenation of a PD controller. The new speeds for the right and left motors are determined 
      based on the current error and the difference between the last error.*/ 
      int16_t speedDifference = 0;
      if (!reached_junction)
      {
        speedDifference = Kp_low * error + Kd_low * (error - lastError);
      }
      else
      {
        speedDifference = Kp_high * error + Kd_high * (error - lastError);
      }
      lastError = error;
      
      // Calculate new speed
      int16_t leftSpeed = (int16_t)maxSpeed + speedDifference;
      int16_t rightSpeed = (int16_t)maxSpeed - speedDifference;
      
      // Make sure the speed is between the acceptable range (0 - 200)
      leftSpeed = constrain(leftSpeed, 0, (int16_t)maxSpeed);
      rightSpeed = constrain(rightSpeed, 0, (int16_t)maxSpeed);
      
      // Set new speeds
      motors.setSpeeds(leftSpeed, rightSpeed);
      
      // Calibration is not needed anymore
      calibration_flag = false;
    }

    // Manual mode
    else{
      
      // Read the third paramter - joyX (X value of the joystick)
      str = "";  
      int ind3 = inputString.indexOf(','); // find location of third delimiter ','
      str = inputString.substring(ind2 + 1, ind3); // capture the joyX substring
      int joyX = str.toInt(); // Convert joyX to integer
      
      // Read the fourth paramter - joyY (Y value of the joystick)
      str = "";
      str = inputString.substring(ind3+1); // capture the joyY substring
      int joyY = str.toInt(); // Convert joyY to integer
      
      // Calculate and set new speeds
      leftMotor = joyY + int(float(joyX)/1.5);
      rightMotor = joyY - int(float(joyX)/1.5);
      motors.setLeftSpeed(leftMotor);
      motors.setRightSpeed(rightMotor);
      
      // Calibration will be needed the next time mode switches to automatic
      calibration_flag = true;
    }
    
    // Assign sensor values into sensor arrays
    getImuSensorValues();
    getLineSensorValues();
    getProxSensorValues();

    // Read battery level
    uint16_t batteryLevel = readBatteryMillivolts();
    float battery = float(batteryLevel)/1000.0;

    // Send data to RaspberryPi via json document
    // data includes timestamp, battery and sensor values which are stored in arrays
    DynamicJsonDocument jBuffer(1024);
    JsonArray line_sensor_values = jBuffer.createNestedArray("line_sensor_values");
    JsonArray imu_sensor_values = jBuffer.createNestedArray("imu_sensor_values");
    JsonArray prox_sensor_values = jBuffer.createNestedArray("prox_sensor_values");
    
    // Populate timestamp and battery
    jBuffer["timestamp"] = timestamp;
    jBuffer["Battery"] = battery;
    
    // Populate sensor values
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

    // Document transmission
    serializeJson(jBuffer, Serial);
    Serial.println();
    
    // Clear the string
    inputString = "";
    stringComplete = false;
  }
}
