// NOTE: MPU interfacing code inspired by https://circuitdigest.com/microcontroller-projects/interfacing-mpu6050-module-with-arduino
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// Enum that we can use to determine which controller we are using
enum CONTROLLER {GYRO, JOYSTICK};
CONTROLLER ctrl;

typedef struct {
  int x;
  int y;
} COORDS;

// Define MPU as global as we will be using it in setup and loop
Adafruit_MPU6050 mpu;

// Bias, set to zero by default
COORDS bias = {.x = 0, .y = 0};

COORDS extractFromGyro();
COORDS extractFromJoystick();

void setup() {
  // Begin serial output on 9600 to communicate with python script
  Serial.begin(9600);
  
  if(mpu.begin()){

    Serial.println("Gyro detected!");
    // MPU was found, game will be played with gyroscope
    ctrl = GYRO;
    
    // set gyro range to +- 2000 deg/s
    mpu.setGyroRange(MPU6050_RANGE_2000_DEG);
    
    // set filter bandwidth to 5 Hz
    mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

    Serial.println("Calibrating gyro, please keep flat...");

    // Delay to give time to keep gyro flat
    delay(1000);

    bias = extractFromGyro();

    Serial.println("Calibration complete!");

  }
  else{

    # TODO: add the joystick init code here

    Serial.println("Gyro not detected!");
    // MPU was not found, game will be played with joystick
    ctrl = JOYSTICK;

  }

  //pinMode(BUZZER, OUTPUT);

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
  
  if (input.x > 170 - bias.x) {
    Serial.println('d');
  } else if (input.x < -170  + bias.x) {
    Serial.println('a');
  }  else if (input.y > 170 - bias.y) {
    Serial.println('w');
  } else if (input.y < -170 + bias.y) {
    Serial.println('s');
  }

  Serial.flush();

  //if (Serial.read() == 'E') {
    //tone(BUZZER, 500); // Beep when an apple is eaten
  //}

  // Delay to control game speed
  delay(100);
}

COORDS extractFromGyro(){

    sensors_event_t a, gyr, t;
    COORDS result;

    mpu.getEvent(&a, &gyr, &t);

    result.x = gyr.gyro.x * 57.2957795;
    result.y = gyr.gyro.y * 57.2957795;

    return result;

}

COORDS extractFromJoystick(){
  COORDS result;

  # TODO: add the gyro code here
  
  result.x = 0;
  result.y = 0;
  
  return result;
}
