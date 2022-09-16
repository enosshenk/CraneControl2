#include <IBusBM.h>

IBusBM IBus;        // IBus object
int Channel[5];     // Array for raw channel data
float ChannelF[5];  // Cleaned up array
bool Direction[5];  // Direction array

// PINS
int M1IN1 = 22; int M1IN2 = 24; int M1IN3 = 26; int M1IN4 = 28; 
int M2IN1 = 30; int M2IN2 = 32; int M2IN3 = 34; int M2IN4 = 36; 
int M3IN1 = 38; int M3IN2 = 40; int M3IN3 = 42; int M3IN4 = 44; 
int M4IN1 = 46; int M4IN2 = 48; int M4IN3 = 50; int M4IN4 = 52; 

int M1ENA = 2;  int M2ENA = 4;  int M3ENA = 6;  int M4ENA = 8;  
int M1ENB = 3;  int M2ENB = 5;  int M3ENB = 7;  int M4ENB = 9;

int RLY1 = 13;

void setup() {
  Serial.begin(115200);
  IBus.begin(Serial2);    // Init on RX2 

  // Init pins
  // PWM
  for (int i=2; i<10; i++){
    pinMode(i, OUTPUT);
    analogWrite(i, 0.0);
  }

  // Digital
  for (int i=22; i<53; i+=2) {
    pinMode(i, OUTPUT);
  }

  // Relay
  pinMode(13, OUTPUT);
}

void loop() {
  
  for (int i=0; i<6; i++) {
    Channel[i] = IBus.readChannel(i);
    Serial.println(Channel[i]);
    if (Channel[i] < 1500) { Direction[i] = false; } else if (Channel[i] > 1500) { Direction[i] = true; }
  }
  
  UpdateModule1();
//  printChannels();

  delay(100);
}

void UpdateModule1() {
 // Serial.println(ChannelF[1]);
  if (Direction[1] == true) {
 //   Serial.println("Forward");
    // Motor 1 forward
    digitalWrite(M1IN1, HIGH );
    digitalWrite(M1IN2, LOW);
    int temp = map(Channel[1], 1000, 1500, 0, 255);
    analogWrite(M1ENA, temp);
  }
  else if (Direction[1] == false) {
  //  Serial.println("Reverse");
    // Motor 1 reverse
    digitalWrite(M1IN1, LOW);
    digitalWrite(M1IN2, HIGH);
    int temp = map(Channel[1], 1500, 2000, 0, 255);
    analogWrite(M1ENA, temp);
  } 
  
  if (Direction[2] == true) {
//    Serial.println("Forward");
    // Motor 2 forward
    digitalWrite(M1IN3, HIGH );
    digitalWrite(M1IN4, LOW);
    int temp = map(Channel[2], 1000, 1500, 0, 255);
    analogWrite(M1ENB, temp);
  }
  else if (Direction[2] == false) {
//    Serial.println("Reverse");
    // Motor 2 reverse
    digitalWrite(M1IN3, LOW);
    digitalWrite(M1IN4, HIGH);
    int temp = map(Channel[2], 1500, 2000, 0, 255);
    analogWrite(M1ENB, temp);
  } 
}

void printChannels() {
  for (int i=0; i<6; i++) {
 //   Serial.print("Channel ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(ChannelF[i]);
  }
}
