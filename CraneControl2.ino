//
//  Tekventure Crane Model
//  Control system
//  by Enos Shenk
//

#include <IBusBM.h>

IBusBM IBus;                              // IBus object
int Channel[7] = {0, 0, 0, 0, 0, 0};     // Array for raw channel data, used for link up/down
int ChannelLast[7] = {0, 0, 0, 0, 0, 0}; // Previous loop channel data
float ChannelF[7];                       // Cleaned up array formatted to -1.0 to 1.0

// Pins
// Motor control direction pins
int M1IN1 = 22; int M1IN2 = 24; int M1IN3 = 26; int M1IN4 = 28; 
int M2IN1 = 30; int M2IN2 = 32; int M2IN3 = 34; int M2IN4 = 36; 
int M3IN1 = 38; int M3IN2 = 40; int M3IN3 = 42; int M3IN4 = 44; 
int M4IN1 = 46; int M4IN2 = 48; int M4IN3 = 50; int M4IN4 = 52; 

// Motor control PWM pins
int M1ENA = 2;  int M2ENA = 4;  int M3ENA = 6;  int M4ENA = 8;  
int M1ENB = 3;  int M2ENB = 5;  int M3ENB = 7;  int M4ENB = 9;

// LEDs and rotation servo
int LightsPin = 10;
int Green = 11;
int Red = 12;

bool LinkUp = false;    // True if transmitter is talking to us
bool Lights = false;    // True if lights are on
int SameReads = 0;      // Counts identical channel reads for determining connection

bool DriveMode = false; // State for driving tracks or controlling crane boom

bool ClawOpen = true;
int ClawInitialTime = 5;
int ClawTimeElapsed = 0;

void setup() {
  Serial.begin(115200);
  IBus.begin(Serial2);    // Init IBus on RX2 

  // Init pins
  // PWM for Module Enable pins
  for (int i=2; i<10; i++){
    pinMode(i, OUTPUT);
    analogWrite(i, 0.0);
  }

  // Digital
  // For module IN1 and IN2 pins
  for (int i=22; i<53; i+=2) {
    pinMode(i, OUTPUT);
  }

  // LED
  pinMode(Green, OUTPUT);
  pinMode(Red, OUTPUT);
  pinMode(LightsPin, OUTPUT);
}

void loop() {
  // Initial linkup
  if (IBus.cnt_rec > 0) { 
    // Count increments at first link
    LinkUp = true; 
  }

  // Transfer current channel data to last array
  for (int i=0; i<7; i++) {
    ChannelLast[i] = Channel[i];     
  }

  // Read current channel data
  for (int i=0; i<7; i++) {
    // Channel[x] is raw servo data, 1000 - 2000 int
    Channel[i] = IBus.readChannel(i);
    // ChannelF[x] is clamped -1.0 - 1.0 float
    float temp = ((float)Channel[i] - 2000.0) / 500.0;
    ChannelF[i] = temp + 1.0;
  }

  // Compare current data to last data
  if (Channel[0] == ChannelLast[0] && 
  Channel[1] == ChannelLast[1] && 
  Channel[2] == ChannelLast[2] && 
  Channel[3] == ChannelLast[3]) {
    // Same data, increment counter
    SameReads++;
  } else {
    // Data changed, we're still linked
    LinkUp = true;
    SameReads = 0;
  }
  if (SameReads > 100) {
    // If we've got a ton of same reads, flag link is down
    LinkUp = false;
  }

  // Control the LED for link status
  if (LinkUp) {
    digitalWrite(Green, HIGH);
    digitalWrite(Red, LOW);
  }
  else
  {
    digitalWrite(Green, LOW);
    digitalWrite(Red, HIGH);
  }

  // Check switch for drive mode toggle
  if (IBus.readChannel(4) < 1300) {
    DriveMode = true;
  }
  else {
    DriveMode = false;
  }

  // Check status of lights
  CheckLights();

  // State change for driving mode
  if (DriveMode) {
    UpdateModule2();      // Only use module 2 for track control
    
  } else {
    analogWrite(4, 0.0);   // Set track motor controls to off
    analogWrite(5, 0.0);
    UpdateModule1();        // Update the rest of the controllers
    UpdateModule3();
    UpdateModule4();
  }

  delay(100);
}

void CheckLights() {
  if (ChannelF[5] > 0.5) {
    Lights = true;
    digitalWrite(LightsPin, HIGH);
  } else {
    Lights = false;
    digitalWrite(LightsPin, LOW);
  }
}


//
//  Functions to update motor control modules below
//  Each module has 4 digital input, two pairs per module
//  Enable pin takes PWM signal
//

void UpdateModule1() {  
  // Boom Rotate
  if (ChannelF[0] > 0.05) {
    // Motor 1 forward
    digitalWrite(M1IN1, HIGH );
    digitalWrite(M1IN2, LOW);
    float temp = ChannelF[0] * 255;
    analogWrite(M1ENA, (int)temp);
  }
  else if (ChannelF[0] < -0.05) {
    // Motor 1 reverse
    digitalWrite(M1IN1, LOW);
    digitalWrite(M1IN2, HIGH);
    float temp = ChannelF[0] * -1;
    analogWrite(M1ENA, temp * 255);
  }
  else
  {
    analogWrite(M1ENA, 0);
  }

  // Side B on module 1 is fucked
/*  if (ChannelF[1] > 0.05) {
    // Motor 2 forward
    digitalWrite(M1IN3, LOW);
    digitalWrite(M1IN4, HIGH);
    float temp = ChannelF[1] * 255;
    analogWrite(M1ENB, (int)temp);
  }
  else if (ChannelF[1] < -0.05) {
    // Motor 2 reverse
    digitalWrite(M1IN3, HIGH);
    digitalWrite(M1IN4, LOW);
    float temp = ChannelF[1] * -1;
    analogWrite(M1ENB, temp * 255);
  }
  else
  {
    analogWrite(M1ENB, 0);
  } */
}

void UpdateModule2() {  
  // Left Track
  if (ChannelF[2] > 0.05) {
    // Motor 3 forward
    digitalWrite(M2IN1, HIGH );
    digitalWrite(M2IN2, LOW);
    float temp = ChannelF[2] * 255;
    analogWrite(M2ENA, (int)temp);
  }
  else if (ChannelF[2] < -0.05) {
    // Motor 3 reverse
    digitalWrite(M2IN1, LOW);
    digitalWrite(M2IN2, HIGH);
    float temp = ChannelF[2] * -1;
    analogWrite(M2ENA, temp * 255);
  }
  else
  {
    analogWrite(M2ENA, 0);
  }

  // Right Track
  if (ChannelF[1] > 0.05) {
    // Motor 4 forward
    digitalWrite(M2IN3, LOW);
    digitalWrite(M2IN4, HIGH);
    float temp = ChannelF[1] * 255;
    analogWrite(M2ENB, (int)temp);
  }
  else if (ChannelF[1] < -0.05) {
    // Motor 4 reverse
    digitalWrite(M2IN3, HIGH);
    digitalWrite(M2IN4, LOW);
    float temp = ChannelF[1] * -1;
    analogWrite(M2ENB, temp * 255);
  }
  else
  {
    analogWrite(M2ENB, 0);
  }
}

void UpdateModule3() { 
    // Claw Grip
  if (ChannelF[5] > 0.05 && ClawOpen) {
    // Close Claw from open state
    ClawOpen = false;
    ClawTimeElapsed = 0;
    UpdateClaw();
  } else if (ChannelF[5] < -0.05 && ClawOpen == false) {
    // Open claw from closed state
    ClawOpen = true;
    ClawTimeElapsed = 0;
    UpdateClaw();
  }

  // Claw Rotate
  if (ChannelF[3] > 0.05) {
    // Motor 4 forward
    digitalWrite(M3IN3, LOW);
    digitalWrite(M3IN4, HIGH);
    float temp = ChannelF[3] * 255;
    analogWrite(M3ENB, (int)temp);
  }
  else if (ChannelF[3] < -0.05) {
    // Motor 4 reverse
    digitalWrite(M3IN3, HIGH);
    digitalWrite(M3IN4, LOW);
    float temp = ChannelF[3] * -1;
    analogWrite(M3ENB, temp * 255);
  }
  else
  {
    analogWrite(M3ENB, 0);
  }
}

void UpdateModule4() {  
  // Boom Extend
  if (ChannelF[2] > 0.05) {
    // Motor 3 forward
    digitalWrite(M4IN1, LOW );
    digitalWrite(M4IN2, HIGH);
    float temp = ChannelF[2] * 255;
    analogWrite(M4ENA, (int)temp);
  }
  else if (ChannelF[2] < -0.05) {
    // Motor 3 reverse
    digitalWrite(M4IN1, HIGH);
    digitalWrite(M4IN2, LOW);
    float temp = ChannelF[2] * -1;
    analogWrite(M4ENA, temp * 255);
  }
  else
  {
    analogWrite(M4ENA, 0);
  }

  // Boom Elevate
  if (ChannelF[1] > 0.05) {
    // Motor 4 forward
    digitalWrite(M4IN3, LOW);
    digitalWrite(M4IN4, HIGH);
    float temp = ChannelF[1] * 255;
    analogWrite(M4ENB, (int)temp);
  }
  else if (ChannelF[1] < -0.05) {
    // Motor 4 reverse
    digitalWrite(M4IN3, HIGH);
    digitalWrite(M4IN4, LOW);
    float temp = ChannelF[1] * -1;
    analogWrite(M4ENB, temp * 255);
  }
  else
  {
    analogWrite(M4ENB, 0);
  }
}

void UpdateClaw() {
  if (ClawOpen) {
    // Open from closed state
    if (ClawTimeElapsed < ClawInitialTime) {
      digitalWrite(M3IN1, LOW);
      digitalWrite(M3IN2, HIGH);
      analogWrite(M3ENA, 64); 
      ClawTimeElapsed++;  
    } else {
      digitalWrite(M3IN1, LOW);
      digitalWrite(M3IN2, HIGH);
      analogWrite(M3ENA, 32);       
    }
  } else {
    // Close from open state
    if (ClawTimeElapsed < ClawInitialTime) {
      digitalWrite(M3IN1, HIGH);
      digitalWrite(M3IN2, LOW);
      analogWrite(M3ENA, 64);  
      ClawTimeElapsed++;
    } else {
      digitalWrite(M3IN1, HIGH);
      digitalWrite(M3IN2, LOW);
      analogWrite(M3ENA, 32);      
    }
  }
}

// Old testing function
void printChannels() {
  for (int i=0; i<6; i++) {
 //   Serial.print("Channel ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(ChannelF[i]);
  }
}
