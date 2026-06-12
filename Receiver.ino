/********************  RECEIVER   *********************/
#include "nRF24L01.h"
#include "RF24.h"
#include "SPI.h"
#include <String.h>

#define MAX_MESSAGE_LEN 32
#define redpin A1
#define greenpin A2
#define bluepin A3
#define Right_IR A4
#define LEFT_IR A5

#define Motor1_R 4
#define Motor1_L 5
#define ENA 7
#define ENB 6
#define Motor2_R 2
#define Motor2_L 3

RF24 radio(9, 10);
const uint64_t pipe = 0xE6E6E6E6E6E6;

char mode;
char directions[MAX_MESSAGE_LEN] = "";
int dirsize = -1;
char ReceivedBuffer[MAX_MESSAGE_LEN] = "";

void setColor(int redValue, int greenValue, int blueValue);
void RF_Init();
void LineFollowing();
void apply_moves(char directions[], int dirsize);
void moveForward();
void moveBackward();
void turnRight();
void turnLeft();
void stopMotors();

void setup(void) {
  Serial.begin(9600);
  pinMode(Right_IR, INPUT);
  pinMode(LEFT_IR, INPUT);
  pinMode(Motor1_R, OUTPUT);
  pinMode(Motor1_L, OUTPUT);
  pinMode(Motor2_R, OUTPUT);
  pinMode(Motor2_L, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  RF_Init();
}

void loop() {
  while (radio.available()) {
    setColor(0, 0, 255); // Blue to show mode change
    memset(ReceivedBuffer, '\0', sizeof(ReceivedBuffer));
    radio.read(ReceivedBuffer, 1);
    Serial.println(ReceivedBuffer);
    mode = (ReceivedBuffer[0] == 0) ? 0 : 1;  // Change mode based on received data

    memset(ReceivedBuffer, '\0', sizeof(ReceivedBuffer));
    if (mode == 1) {
      Serial.println("code receiving");
      radio.read(ReceivedBuffer, sizeof(ReceivedBuffer));
      strcpy(directions, ReceivedBuffer);
      dirsize = strlen(ReceivedBuffer);
      apply_moves(directions, dirsize);  // Execute movements based on received directions
    } 
    else if (mode == 0) {
      Serial.println("ir mode");
      LineFollowing();  // Start line-following mode
    }
  }
}

/********************  LED FUNCTION   ***********************/
void setColor(int redValue, int greenValue, int blueValue) {
  analogWrite(redpin, redValue);
  analogWrite(greenpin, greenValue);
  analogWrite(bluepin, blueValue);
}

/********************  RF INIT FUNCTIONS   ***********************/
void RF_Init() {
  while (!radio.begin()) {
    setColor(255, 0, 0); // Red on failure
    Serial.println("Radio initialization failed!");
  }
  setColor(0, 255, 0); // Green on success
  radio.openReadingPipe(1, pipe);
  radio.startListening();
  Serial.println("Radio initialized");
}

/********************  LINE FOLLOWING FUNCTION   ***********************/
void LineFollowing() {
  int leftSensorValue, rightSensorValue;
  int threshold = 500;

  while (mode == 0) {
    leftSensorValue = analogRead(LEFT_IR);
    rightSensorValue = analogRead(Right_IR);

    // Decide action based on sensor values
    if (leftSensorValue < threshold && rightSensorValue < threshold) moveForward();
    else if (leftSensorValue > threshold && rightSensorValue < threshold) turnRight();
    else if (rightSensorValue > threshold && leftSensorValue < threshold) turnLeft();
    else stopMotors();

    // Check if mode has changed
    if (radio.available()) {
      setColor(0, 0, 255);  // Change color to show new command
      memset(ReceivedBuffer, '\0', sizeof(ReceivedBuffer));
      radio.read(ReceivedBuffer, 1);
      mode = (ReceivedBuffer[0] == 0) ? 0 : 1; // Change mode based on received data
      break;  // Exit if mode changes
    }
    delay(100); // Delay for smoother control
  }
}

/********************  MOTOR CONTROL FUNCTIONS   ***********************/
void apply_moves(char directions[], int dirsize) {
  Serial.print("Applying moves from directions: ");
  for (int i = 0; i < dirsize; i++) {
    char next_move = directions[i];
    switch (next_move) {
      case 'f': moveForward(); delay(500); stopMotors(); break;
      case 'b': moveBackward(); delay(500); stopMotors(); break;
      case 'r': turnRight(); delay(300); stopMotors(); break;
      case 'l': turnLeft(); delay(300); stopMotors(); break;
      default: Serial.println("Unknown command"); stopMotors(); break;
    }
  }
}

/********************  MOTOR CONTROL FUNCTIONS   ***********************/
void moveForward() {
  digitalWrite(ENA, HIGH);
  digitalWrite(Motor1_R, HIGH);
  digitalWrite(Motor1_L, LOW);
  digitalWrite(ENB, HIGH);
  digitalWrite(Motor2_R, HIGH);
  digitalWrite(Motor2_L, LOW);
}

void moveBackward() {
  digitalWrite(ENA, HIGH);
  digitalWrite(Motor1_R, LOW);
  digitalWrite(Motor1_L, HIGH);
  digitalWrite(ENB, HIGH);
  digitalWrite(Motor2_R, LOW);
  digitalWrite(Motor2_L, HIGH);
}

void turnRight() {
  digitalWrite(ENA, HIGH);
  digitalWrite(Motor1_R, LOW);
  digitalWrite(Motor1_L, HIGH);
  digitalWrite(ENB, HIGH);
  digitalWrite(Motor2_R, HIGH);
  digitalWrite(Motor2_L, LOW);
}

void turnLeft() {
  digitalWrite(ENA, HIGH);
  digitalWrite(Motor1_R, HIGH);
  digitalWrite(Motor1_L, LOW);
  digitalWrite(ENB, HIGH);
  digitalWrite(Motor2_R, LOW);
  digitalWrite(Motor2_L, HIGH);
}

void stopMotors() {
  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);
}
