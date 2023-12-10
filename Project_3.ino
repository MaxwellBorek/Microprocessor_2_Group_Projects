#include "arduinoFFT.h"
#include "uRTCLib.h"
#include <LiquidCrystal.h>

#define NUM_SAMPLES 128                // Number of points for FFT. Must be a power of 2. Max 128 for Arduino Uno.
#define SAMPLING_FREQ 2100             // Sampling frequency, based on Nyquist theorem (must be at least 2 times the highest expected frequency).

// uRTCLib rtc;
uRTCLib rtc(0x68);

arduinoFFT fft = arduinoFFT();         // Create an instance of the arduinoFFT library.

unsigned int samplingPeriod;
unsigned long currentTime;

double realValues[NUM_SAMPLES];       // Array to hold real values of the signal.
double imagValues[NUM_SAMPLES];       // Array to hold imaginary values of the signal.

int motorIn1 = 10;
int motorIn2 = 9;
int motorSpeed = 8;

String rotation = "C";
String speedStr = "0";
int speed;
int timeHour;
int timeMinute;
char str[16];
int domFrequency;

//Initialize LCD pins: LCD RS pin to digital pin 12, LCD Enable pin to digital pin 11, LCD D4 pin to digital pin 5
//LCD D5 pin to digital pin 4, LCD D6 pin to digital pin 3, and LCD D7 pin to digital pin 2
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

String temp;

// Interrupt code inspired by: https://deepbluembedded.com/arduino-timer-interrupts/
void interruptSetup(){

  TCCR1A = 0;           // Init Timer1
  TCCR1B = 0;           // Init Timer1
  TCCR1B |= B00000011;  // Prescalar = 64

  // Value of 250000 creates an interrupt every second given the prescalar of 64 and the frequency
  OCR1A = 250000;        // Timer CompareA Register

  TIMSK1 |= B00000010;  // Enable Timer COMPA Interrupt

}

void setup() {

  // Setup pins
  pinMode(motorIn1, OUTPUT);
  pinMode(motorIn2, OUTPUT);
  pinMode(A8, OUTPUT);

  // Setup motor
  digitalWrite(motorIn1, LOW);
  digitalWrite(motorIn2, LOW);
  analogWrite(motorSpeed, 0);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // Setup serial connection
  Serial.begin(9600);

  // Setup wire
  URTCLIB_WIRE.begin();

  //rtc.set(second, minute, hour, dayOfWeek, dayOfMonth, month, year)
  rtc.set(0, 12, 2, 5, 7, 12, 23);
  //rtc.enableBattery();

  // Setup an interrupt for every second
  interruptSetup();

  samplingPeriod = round(1000000.0 / SAMPLING_FREQ); // Calculate sampling period in microseconds.

}

void loop() {

  rtc.refresh();

  for (int i = 0; i < NUM_SAMPLES; i++) {
    currentTime = micros();       // Record the current time.

    realValues[i] = analogRead(0); // Read the value from analog pin 0 (A0) and store it as a real term.
    imagValues[i] = 0;            // Set the imaginary term to 0.

    // Wait until the next sampling period.
    while (micros() < (currentTime + samplingPeriod)) {
        // Do nothing - wait for the sampling period to elapse.
    }
  }

  // Perform FFT on the collected samples.
  fft.Windowing(realValues, NUM_SAMPLES, FFT_WIN_TYP_BLACKMAN_HARRIS, FFT_FORWARD);
  fft.Compute(realValues, imagValues, NUM_SAMPLES, FFT_FORWARD);
  fft.ComplexToMagnitude(realValues, imagValues, NUM_SAMPLES);

  // Find the dominant frequency and print it.
  double dominantFrequency = fft.MajorPeak(realValues, NUM_SAMPLES, SAMPLING_FREQ);
  domFrequency = (int)dominantFrequency;
  //Serial.println(dominantFrequency); // Print out the most dominant frequency.
  //speed 0 -> 1/2
  if((domFrequency >= 257) && (domFrequency <= 267) && (speedStr == "0"))
  {
    speedStr = "1/2";
    speed = 127;
    //lcd.print(" 1/2 ");
  }
  //speed 1/2 -> 3/4
  else if(domFrequency >= 257 && domFrequency <= 267 && speedStr == "1/2")
  {
    speedStr = "3/4";
    speed = 191;
    //lcd.print(" 3/4 ");
  }
  //speed 3/4 -> Full
  else if(domFrequency >= 257 && domFrequency <= 267 && speedStr == "3/4")
  {
    speedStr = "Full";
    speed = 255;
    //lcd.print(" Full ");
  }
  //speed Full -> Full
  else if(domFrequency >= 257 && domFrequency <= 267 && speedStr == "Full")
  {
    speedStr = "Full";
    speed = 255;
    //lcd.print(" Full");
  }
  //speed Full -> 3/4
  if((domFrequency > 431) && (domFrequency) < 449 && (speedStr == "Full"))
  {
    speedStr = "3/4";
    speed = 191;
    //lcd.print(" 3/4 ");
  }
  //speed 3/4 -> 1/2
  else if((domFrequency > 431) && (domFrequency < 449) && (speedStr == "3/4"))
  {
    speedStr = "1/2";
    speed = 127;
    //lcd.print(" 1/2 ");
  }
  //speed 1/2 -> 0
  else if((domFrequency > 431) && (domFrequency < 449) && (speedStr == "1/2"))
  {
    speedStr = "0";
    speed = 0;
    //lcd.print(" 0 ");
  }
  //C -> CC
  if((domFrequency > 431) && (domFrequency < 449) && (speedStr == "0") && (rotation == "C"))
  {
    rotation = "CC";
    speedStr = "0";
    speed = 0;
    //lcd.print(" 0 ");
  }
  //CC -> C
  else if((domFrequency > 431) && (domFrequency < 449) && (speedStr == "0") && (rotation == "CC"))
  {
    rotation = "C";
    speedStr = "0";
    speed = 0;
    //lcd.print(" 0 ");
  }

  if(rotation == "C")
  {
    digitalWrite(motorIn1, HIGH);
    digitalWrite(motorIn2, LOW);
    //lcd.print("C");
  }
  else if(rotation == "CC")
  {
    digitalWrite(motorIn1, LOW);
    digitalWrite(motorIn2, HIGH);
    //lcd.print("CC");
  }
  //set motor speed
  Serial.println(speed);
  analogWrite(8, speed);

  timeHour = rtc.hour();
  timeMinute = rtc.minute();

  delay(1000); // Delay for visualization purposes or other operations.
  // Script stops here. Hardware reset might be required for further execution.
}

// Executes every second
ISR(TIMER1_COMPA_vect)
{
  
  temp = String(rtc.hour()) + ":" + String(rtc.minute()) + " " + String(rotation) + " " + speedStr + " ";
  lcd.print(temp);
  lcd.setCursor(0, 0);

}