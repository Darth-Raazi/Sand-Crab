// Rx Version 6
// Ozayr Raazi
// April 27th, 2022

// Include required libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// NRF24l01 SPI communications pins
#define CE_PIN  7 // for Uno, check wiring
#define CSN_PIN 8 // for Uno, check wiring

// Adafruit servo board config
#define SERVOMIN  190 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  410 // This is the 'maximum' pulse length count (out of 4096)
#define SERVOMID  300 // This is the 'middle' pulse length count (out of 4096)
#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

// NRF pipe address
const byte thisSlaveAddress = '0xF0F0F0F0F0';

// Create a radio
RF24 radio(CE_PIN, CSN_PIN);

// Package structure for data
struct package
{
  char S = 'N';
  char A = 'N';
  char C = 'N';
  char T = 'N';
  int X;
  int Y;
};
typedef struct package Package;
Package data;

// Main motor outputs
int OUT1 = 5;
int OUT2 = 6;

// Movement values
int Xval;
int Yval;
int forward;
int backward;
int turnmap;
int turn;
int rev;
int servopos = 123;

// Data values
char Start;
char Arm;
char Claw;
char Crab;
bool newData = false;
int armAngle = SERVOMIN;
int clawAngle = SERVOMIN;

int counter = 0;

//===========

void setup() {
    Serial.begin(9600);
    Serial.println("Rx6 Starting");
    
    // Initialize radio
    radio.begin();
    radio.setPALevel(RF24_PA_MAX);
    radio.setDataRate( RF24_250KBPS );
    radio.openReadingPipe(1, thisSlaveAddress);
    radio.startListening();
    Serial.println("Listening");

    // Standby until start command is received
    do {
      getData();
    }
    while (data.S != 'Y');
    
    // Initialize motor and servos
    pinMode(OUT1, OUTPUT);
    pinMode(OUT2, OUTPUT);
    analogWrite(OUT1, 0);
    analogWrite(OUT2, 0);
    pwm.begin();
    pwm.setOscillatorFrequency(27000000);
    pwm.setPWMFreq(SERVO_FREQ);
    pwm.setPWM(0, 0, SERVOMID);
    pwm.setPWM(1, 0, SERVOMID);
    pwm.setPWM(2, 0, SERVOMIN);
    pwm.setPWM(3, 0, SERVOMIN);
    Serial.println("Initialized");
}

//=============

void loop() {
  // Standby mode
  do {
    getData();
  }
  while (Start != 'Y');

  // Turning 
  if (Xval > 600 || Xval < 400) {
    turnmap = map(Xval, 0, 1023, 0, 180);
    turn = map(turnmap, 0, 180, SERVOMIN, SERVOMAX);
    rev = 300 - (turn-300)/2;

    pwm.setPWM(0, 0, turn);
    if (Crab == 'Y') {
        pwm.setPWM(1, 0, rev);
    }
  } else {
    pwm.setPWM(0, 0, SERVOMID);
    pwm.setPWM(1, 0, SERVOMID);
  }

  // Forward or Backward
  if (Yval > 500) {
    forward = map(Yval, 500, 1023, 0, 255);
    analogWrite(OUT1, forward);
    analogWrite(OUT2, 0);
  } else if (Yval < 480) {
    backward = map(Yval, 480, 0, 0, 255);
    analogWrite(OUT1, 0);
    analogWrite(OUT2, backward);
  } else {
    analogWrite(OUT1, 0);
    analogWrite(OUT2, 0);
  }

  // Arm and claw movement
  if (Arm != 'N') {
      if (Arm == 'U') {
          armAngle += 10;
          pwm.setPWM(2, 0, armAngle);
      }
      else {
          armAngle -= 10;
          pwm.setPWM(2, 0, armAngle);
      }
  }
  
  if (Claw != 'N') {
      if (Claw == 'O') {
          clawAngle += 10;
          pwm.setPWM(3, 0, clawAngle);
      }
      else {
          clawAngle -= 10;
          pwm.setPWM(3, 0, clawAngle);
      }
  }
}

//==============

void getData() {
  if (radio.available()) {
      radio.read( &data, sizeof(data) );
      counter++;
      Xval = data.X;
      Yval = data.Y;
      Start = data.S;
      Arm = data.A;
      Claw = data.C;
      Crab = data.T;
      newData = true;
      // For debugging
      //showData();
  }
}

void showData() {
    if (newData == true) {
        Serial.print("Data received ");
        Serial.print("X:");
        Serial.print(Xval);
        Serial.print("  Y:");
        Serial.print(Yval);
        Serial.print("  S:");
        Serial.print(Start);
        Serial.print("  A:");
        Serial.print(Arm);
        Serial.print("  C:");
        Serial.print(Claw);
        Serial.print("  T:");
        Serial.println(Crab);
        newData = false;
        Serial.print("Size: ");
        Serial.println(sizeof(data));
        Serial.print("Counter: ");
        Serial.println(counter);

    }
    
}
