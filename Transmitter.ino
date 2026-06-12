/********************  TRANSMITTER   *********************/
#include "nRF24L01.h" 
#include "RF24.h"
#include "SPI.h"
#include <String.h>
#define MAX_MESSAGE_LEN 32

#define redpin A1
#define greenpin A2
#define bluepin A3

#define MODE 8
#define PB_FORWARD 5
#define PB_BACKWARD 4
#define PB_RIGHT 7
#define PB_LEFT 6
#define PB_GO 3
#define PB_CLEAR 2

RF24 radio(9, 10); // NRF24L01 used SPI pins + Pin 9 and 10 on the NANO
const uint64_t pipe = 0xE6E6E6E6E6E6; 

int mode_status;
char directions[MAX_MESSAGE_LEN] = "";  
int dirsize = -1;
char go = 0;
char Clr = 0;
char dirinput;

unsigned long lastDebounceTimeForward = 0;
unsigned long lastDebounceTimeBackward = 0;
unsigned long lastDebounceTimeRight = 0;
unsigned long lastDebounceTimeLeft = 0;
unsigned long lastDebounceTimeGo = 0;
unsigned long lastDebounceTimeClear = 0;
const unsigned long debounceDelay = 200;
uint8_t prevForwardState = HIGH;
uint8_t prevBackwardState = HIGH;

void setColor(int redValue, int greenValue, int blueValue);
void RF_Init();
void Get_Directions();

void setup() {
  Serial.begin(9600); 
  RF_Init();
  pinMode(PB_FORWARD, INPUT_PULLUP);
  pinMode(PB_BACKWARD, INPUT_PULLUP);
  pinMode(PB_RIGHT, INPUT_PULLUP);
  pinMode(PB_LEFT, INPUT_PULLUP);
  pinMode(PB_GO, INPUT_PULLUP);
  pinMode(PB_CLEAR, INPUT_PULLUP);
  pinMode(MODE, INPUT); 
}

void loop() {
  mode_status = digitalRead(MODE);
  bool success = radio.write((char)mode_status, sizeof((char)mode_status));
  Serial.print(" MODE = ");
  Serial.println(mode_status);
  if (mode_status == 0) {
    delay(1000);
  }
  while (mode_status == 1) {
    Get_Directions();
    mode_status = digitalRead(MODE);
  }
}

/********************  LED FUNCTION   ***********************/
void setColor(int redValue, int greenValue, int blueValue) {
    analogWrite(redpin, redValue);
    analogWrite(greenpin, greenValue);
    analogWrite(bluepin, blueValue);
}

/********************  RF INIT FUNCTIONS   ***********************/
void RF_Init(){
  while (!radio.begin()) {
    setColor(255, 0, 0);
    Serial.println("Radio initialization failed!");
  }
  radio.openWritingPipe(pipe); 
  setColor(0, 255, 0);
  Serial.println("Radio initialized");
}

/****************** TAKING DIRECTIONS**************************/
void Get_Directions(){
  unsigned long currentMillis = millis();
  uint8_t forward = digitalRead(PB_FORWARD);
  uint8_t backward = digitalRead(PB_BACKWARD);
  uint8_t right = digitalRead(PB_RIGHT);
  uint8_t left = digitalRead(PB_LEFT);
  uint8_t goState = digitalRead(PB_GO);
  uint8_t erase = digitalRead(PB_CLEAR);
  
  if (erase == LOW && (currentMillis - lastDebounceTimeClear > debounceDelay)) {
    memset(directions, '\0', sizeof(directions));
    dirsize = -1;
    Serial.println("Directions Cleared");
    lastDebounceTimeClear = currentMillis;
  }

  if (goState == LOW && (currentMillis - lastDebounceTimeGo > debounceDelay)) {
    Serial.print("Sending characters:");
    Serial.println(directions);
    for (int i = 0; i <= 3; i++) {
    //bool success = radio.write(&directions[i], 1);
      bool success = radio.write(&directions, 1);
      if (!success) {
        Serial.println("Transmission failed. Retrying...");
        setColor(255, 0, 0);
        delay(200);
        setColor( 0,255, 0);
      } else {
        Serial.println("Command sent: " + String(directions));
      }
    }
    memset(directions, '\0', sizeof(directions));
    dirsize = -1;
    lastDebounceTimeGo = currentMillis;
  }

  if (goState == HIGH) {
    currentMillis = millis();  
    if (forward == LOW && (currentMillis - lastDebounceTimeForward > debounceDelay)) {
      directions[++dirsize] = 'f';
      Serial.println("f");
      lastDebounceTimeForward = currentMillis;
    }
    if (backward == LOW && (currentMillis - lastDebounceTimeBackward > debounceDelay)) {
      directions[++dirsize] = 'b';
      Serial.println("b");
      lastDebounceTimeBackward = currentMillis;
    }
    if (right == LOW && (currentMillis - lastDebounceTimeRight > debounceDelay)) {
      directions[++dirsize] = 'r';
      Serial.println("r");
      lastDebounceTimeRight = currentMillis;
    }
    if (left == LOW && (currentMillis - lastDebounceTimeLeft > debounceDelay)) {
      directions[++dirsize] = 'l';
      Serial.println("l");
      lastDebounceTimeLeft = currentMillis;
    }
  }
}
