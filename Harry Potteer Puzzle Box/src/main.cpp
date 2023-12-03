/*
Harry Potter Puzzle Box

Author: Robert Matania
Date: 2023

*/
#include <Arduino.h>
#include "EEPROM.h"

#define DEBUG true

#define VERSION 0.1
#define STATE_ADDRESS 0


enum puzzleStates {                         // Define enumerated type for state machine states
  NONE,
  IDLE, 
  STATE_1, 
  STATE_2, 
  STATE_3,
  STATE_4,
  STATE_5,
  STATE_6,
  STATE_7
};

byte savedState;
puzzleStates lastState, currentState;

void idle() {
// If entering the state, do initialization stuff
  if (currentState != lastState) {         
    lastState = currentState;
   
  }

// Perform state tasks

// Check for state transitions

// If leaving the state, do clean up stuff
 if (currentState != lastState) {         // If we are leaving the state, do clean up stuff
    
  }

}


void state_1() {

}

void state_2() {

}

void state_3() {

}

void state_4() {

}

void state_5() {

}

void state_6() {

}





void setup() {
  
  Serial.begin(9600);
  
  savedState = EEPROM.read(STATE_ADDRESS);

  if (savedState == 0xFF)
    savedState = IDLE;
  currentState = puzzleStates(savedState);
  lastState = NONE;

  #ifdef DEBUG
   Serial.print(F("Version: "));
   Serial.println(VERSION);
  #endif

  
  
}

void loop(){
  switch (currentState) {
    
    case NONE:
      break;

    case IDLE:
      idle();
      break;

    case STATE_1:
      state_1();
      break;
  
    case STATE_2:
      state_2();
      break;
  
    case STATE_3:
      state_3();
      break;
  
    default:
      break;
  }
 
}