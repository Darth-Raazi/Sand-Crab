// Tx Version 6
// Ozayr Raazi
// April 27th, 2022

// Keep Funduino voltage switch at 5V

// Include required libraries
#include <SPI.h>       
#include <nRF24L01.h>
#include <RF24.h>

// NRF24L01 SPI communication pins
#define CE_PIN   9 // for Funduino, check wiring      
#define CSN_PIN 10 // for Funduino, check wiring

// Joystick pins
#define X A0            
#define Y A1
#define Size 7

// Buttons
int Start = 7;
int Stop = 6;
int armup = 2;
int armdown = 4;
int clawopen = 3;
int clawclose = 5;
int crab = 8;
int buttons[Size] = {Start, Stop, armup, armdown, clawopen, clawclose, crab};          

// NRF pipe address
const byte slaveAddress = '0xF0F0F0F0F0';

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
  int Z = 300;
};
typedef struct package Package;
Package data;

int counter = 0;

//===================

void setup() {
  Serial.begin(9600);
  Serial.println("Tx6 Starting");
  
  // Initialize buttons and joystick
  for (int i; i < Size; i++) { 
    pinMode(buttons[i], INPUT);
    digitalWrite(buttons[i], HIGH);
  }

  pinMode(X, INPUT);
  pinMode(Y, INPUT);
  
  // Initialize radio
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MIN);
  radio.openWritingPipe(slaveAddress);

  // Standby until user presses Start
  while (digitalRead(Start) != LOW);
  data.S = 'Y';
  send();
  Serial.print("Initalized");
}

//====================

void loop() {
  // Toggle standby mode
  if (digitalRead(Stop) == LOW && digitalRead(Start) == HIGH) {
    data.S = 'N';
  } else if (digitalRead(Start) == LOW && digitalRead(Stop) == HIGH) {
    data.S = 'Y';
  }

  // Drive forwards or backwards
  data.X = analogRead(X);
  data.Y = analogRead(Y);

  // Raise or lower arm
  if (digitalRead(armup) == LOW && digitalRead(armdown) == HIGH) {
    data.A = 'U';
  } else if (digitalRead(armdown) == LOW && digitalRead(armup) == HIGH) {
    data.A = 'D';
  } else {
    data.A = 'N';
  }

  // Open or close claw
  if (digitalRead(clawopen) == LOW && digitalRead(clawclose) == HIGH) {
    data.C = 'O';
  } else if (digitalRead(clawclose) == LOW && digitalRead(clawopen) == HIGH) {
    data.C = 'C';
  } else {
    data.C = 'N';
  }

  // Toggle crab steering
  if (digitalRead(crab) == LOW) {
    if (data.T == 'N') {
      data.T = 'Y';
    } else {
      data.T = 'N';
    }
  }
  
  // Send data
  send();
}

//====================

void send() {
    bool rslt;
    rslt = radio.write(&data, sizeof(data));
    counter++;

    // for debugging
    Serial.print("Data Sent X:");
    Serial.print(data.X);
    Serial.print("  Y:");
    Serial.print(data.Y);
    Serial.print("  S:");
    Serial.print(data.S);
    Serial.print("  A:");
    Serial.print(data.A);
    Serial.print("  C:");
    Serial.print(data.C);
    Serial.print("  T:");
    Serial.print(data.T);
    
    if (rslt) {
        Serial.println("  Acknowledge received");
    }
    else {
        Serial.println("  Tx failed");
    }

    Serial.print("Counter: ");
    Serial.println(counter);
}
