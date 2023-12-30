/*
Harry Potter Puzzle Box

Author: Robert Matania
Date: 2023

*/
#include <Arduino.h>
#include <EEPROM.h>
#include <Servo.h>
#include <PCF8574.h>
#include <DYPlayerArduino.h>
#include <Ticker.h>
#include <HPPuzzleBox.h>

#define DEBUG true

#define VERSION 0.1
#define EPROM_STATE_ADDRESS 0


enum puzzleStates {                         // Define enumerated type for state machine states
  NONE,
  INIT,
  STATE_0, // Lid Closed
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


void init() {

// If entering the state, do initialization stuff
  if (currentState != lastState) {         
    lastState = currentState;

  savedState = EEPROM.read(EPROM_STATE_ADDRESS);

  if (savedState == 0xFF)
    savedState = NONE;
  
/*
  currentState = puzzleStates(savedState);
  lastState = NONE;
*/  
   
  }

// Perform state tasks

// Check for state transitions

// If leaving the state, do clean up stuff
 if (currentState != lastState) {         // If we are leaving the state, do clean up stuff
    
  }
}
// -------------------------------------------------------------------------------------
// Initial Game state.
// Lid closed, all traps closed.

void state_0() {
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
// -------------------------------------------------------------------------------------
void state_1() {

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
// -------------------------------------------------------------------------------------
void state_2() {
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
// -------------------------------------------------------------------------------------
void state_3() {
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
// -------------------------------------------------------------------------------------
void state_4() {
// If entering the state, do initialization stuff
  if (currentState != lastState) {         
    lastState = currentState;
   
  }

// Perform state tasks

// Check for state transitions

// If leaving the state, do clean up stuff
 if (currentState != lastState) {         
    
  }
}
// -------------------------------------------------------------------------------------
void state_5() {
// If entering the state, do initialization stuff
  if (currentState != lastState) {         
    lastState = currentState;
   
  }

// Perform state tasks

// Check for state transitions

// If leaving the state, do clean up stuff
 if (currentState != lastState) {        

  }
}
// -------------------------------------------------------------------------------------
void state_6() {
// If entering the state, do initialization stuff
  if (currentState != lastState) {         
    lastState = currentState;
   
  }

// Perform state tasks

// Check for state transitions

// If leaving the state, do clean up stuff
 if (currentState != lastState) {       
    
  }
}


// -------------------------------------------------------------------------------------

void setup() {
  
  Serial.begin(9600);

  currentState = INIT;
  lastState = NONE;
  
  
   Serial.print(F("Version: "));
   Serial.println(VERSION);
  
}

void loop(){
  switch (currentState) {

    case NONE:
      break;

    case INIT:
      init();
      break;

    case STATE_0:
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