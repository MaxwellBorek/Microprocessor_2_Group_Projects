// NOTE: MPU interfacing code inspired by https://circuitdigest.com/microcontroller-projects/interfacing-mpu6050-module-with-arduino
// We will be using the Adafruit MPU library to take care of i2c communication with the MPU
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// Define the pin out for the buzzer
#define BUZZER 9

// Enum that we can use to determine which controller we are using
enum CONTROLLER {GYRO, JOYSTICK};

// Global controller type variable
CONTROLLER ctrl;

// Coordinates structure
typedef struct {
  int x;
  int y;
} COORDS;

// Bias for gyroscope, set to zero by default
COORDS bias = {.x = 0, .y = 0};

// Define MPU as global as we will be using it in setup and loop
Adafruit_MPU6050 mpu;

// Function declarations
COORDS initGyro();
void initJoystick();
COORDS extractFromGyro();
COORDS extractFromJoystick();

void setup() {
  // Begin serial output on 9600 to communicate with python script
  Serial.begin(9600);

  // Setup buzzer
  pinMode(BUZZER, OUTPUT);

  // Init controller
  if(mpu.begin()){

    initGyro();
    Serial.println("Calibration complete!");

  }
  else{

    Serial.println("Gyro not detected!");
    // MPU was not found, game will be played with joystick
    initJoystick();

  }

  delay(200);
}

void loop() {

  COORDS input;

  // Get controller input
  if(ctrl == GYRO){
    input = extractFromGyro();
  }
  else{
    input = extractFromJoystick();
  }

  // Determine key value
  if (input.x > 170 - bias.x) {
    Serial.println('d');
  } else if (input.x < -170 + bias.x) {
    Serial.println('a');
  }
  
  if (input.y > 170 - bias.y) {
    Serial.println('w');
  } else if (input.y < -170 + bias.y) {
    Serial.println('s');
  }

  // Make sure that the value is printed to serial
  Serial.flush();

  // Set off buzzer if apple is eaten
  if (Serial.read() == 'E') {
    tone(BUZZER, 500, 500); // Beep when an apple is eaten
  }

  // Delay to control game speed
  delay(100);
}

// Initializes the gyroscope and returns the default X and Y when the gyro is flat
COORDS initGyro(){
    Serial.println("Gyro detected!");
    // MPU was found, game will be played with gyroscope
    ctrl = GYRO;
    
    // set gyro range to +- 500 deg/s
    mpu.setGyroRange(MPU6050_RANGE_2000_DEG);
    
    // set filter bandwidth to 5 Hz
    mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

    Serial.println("Calibrating gyro, please keep flat...");

    // Delay to give time to keep gyro flat
    delay(1000);

    return extractFromGyro();
}

// Initializes the joystick pins
void initJoystick(){

  ctrl = JOYSTICK;
  
  // Init analog pins as input
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  
}

// Extracts current X and Y values from the gyroscope and returns result as degrees/second
COORDS extractFromGyro(){

    sensors_event_t a, gyr, t;
    COORDS result;

    mpu.getEvent(&a, &gyr, &t);

    result.x = gyr.gyro.x * 57.2957795;
    result.y = gyr.gyro.y * 57.2957795;

    return result;

}

// Extracts current X and Y values from the joystick and returns result
// Joystick outputs from 0 to 1024, function converts to similar number system as gyroscope
COORDS extractFromJoystick(){
  COORDS result;

  result.x = analogRead(A0) - 500;
  result.y = - (analogRead(A1) - 500);
  
  return result;
}
