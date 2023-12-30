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
#include <PinChangeInterrupt.h>
#include <Ticker.h>
#include <HPPuzzleBox.h>

#define NODEBUG 
#define DEFAULT_VOLUME MIN_VOLUME
#define SOUND_ONOFF  1 // 0 = off, 1 = on
#define VERSION 0.1
#define EPROM_STATE_ADDRESS 0

#define adressPCF8574_0 0x20 
#define adressPCF8574_1 0x21

void pcf0_InterruptionRoutine();
void pcf1_InterruptionRoutine();

PCF8574::DigitalInput di_0;
PCF8574::DigitalInput di_1;

bool managePcfInterrupt_0 = false;
bool managePcfInterrupt_1 = false;

PCF8574 pcf8574_0(adressPCF8574_0,3,pcf0_InterruptionRoutine);
PCF8574 pcf8574_1(adressPCF8574_1,2,pcf1_InterruptionRoutine);


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
int mp3Busy = 1; // 0=playing; 1=not playing, 
int helpBtn;

byte newPuckStates;
byte lastPuckStates = 0;
int lidState;



DY::Player player(&Serial2);


//--------------------------------------------------------------------------------------
int readMp3State(void){
  int mp3State = digitalRead(MP3_BUSY);
  return mp3State;
}
// -------------------------------------------------------------------------------------
void stopMp3Play(void){

  player.stop();

}
// -------------------------------------------------------------------------------------
void startMp3Play(int index,int vol)
{
  if (SOUND_ONOFF == 1){
    player.setVolume(vol); 
    player.playSpecified(index);
   // Serial.println("Start Playing");
  }
}
//--------------------------------------------------------------------------------------
byte readPuckStates(bool playSound){
  byte tmpPuckState;

  tmpPuckState = digitalRead(PUCK_1);
  newPuckStates = tmpPuckState;
  tmpPuckState = digitalRead(PUCK_2);
  newPuckStates = newPuckStates | tmpPuckState << 1;
  tmpPuckState = digitalRead(PUCK_3);
  newPuckStates= newPuckStates | tmpPuckState << 2;
  tmpPuckState = digitalRead(PUCK_4);
  newPuckStates = newPuckStates | tmpPuckState << 3;

  if (newPuckStates < lastPuckStates){
    Serial.print(lastPuckStates);  
    Serial.print(" ");  
    Serial.println(newPuckStates);  
    if (playSound)
      startMp3Play(2,DEFAULT_VOLUME);
  }
  lastPuckStates = newPuckStates;

  return newPuckStates;
  }
//--------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------

void state_init() {

// If entering the state, do initialization stuff
  if (currentState != lastState) {     
    Serial.print(F("Enter State: "));
    Serial.println("INIT");    

    lastState = currentState;


  if (helpBtn == 0) {
    EEPROM.write(EPROM_STATE_ADDRESS,0xFF);
    Serial.println("Resetting EEPROM"); 
  }
  savedState = EEPROM.read(EPROM_STATE_ADDRESS);
  Serial.print(F("Saved State: "));
  Serial.println(savedState); 

  if (savedState == 0xFF)
    savedState = STATE_0; // First time
  
  currentState = puzzleStates(savedState);
  Serial.print(F("Current State: "));
  Serial.println(currentState); 
     
  }

// Perform state tasks
// Neve gets here

// Check for state transitions

// If leaving the state, do clean up stuff
 if (currentState != lastState) {         // If we are leaving the state, do clean up stuff
    
  }
}
// -------------------------------------------------------------------------------------

void state_0() {
// Initial Game state.
// Play generic
// All traps closed but don't know if trap 1 is open (padlock).
// Monitor cogs for trap_2 trigger and then move to state_1 
// Monitor pucks and play sound when detected


// If entering the state, do initialization stuff
  if (currentState != lastState) {     
    EEPROM.write(EPROM_STATE_ADDRESS,currentState);   

    Serial.print(F("Enter State: "));
    Serial.println(0);
    lastState = currentState;

    startMp3Play(1,DEFAULT_VOLUME);
  }
Serial.print(mp3Busy);
Serial.print(" ");
Serial.println(lidState);
delay(500);

// Perform state tasks
// Check for state transitions
    mp3Busy = readMp3State();
    if ((lidState == 1) && (mp3Busy == 0))
      stopMp3Play();

#ifdef DEBUG
Serial.print(F("mp3 State: "));
Serial.println(mp3Busy);
#endif

  readPuckStates(true); // play sound in puck is inserted
  
  if (managePcfInterrupt_0) {
    managePcfInterrupt_0 = false;
    Serial.println("PCF_0: ");
    di_0 = pcf8574_0.digitalReadAll();
    Serial.print(di_0.p0, BIN);    
    Serial.print(di_0.p1, BIN); 
    Serial.print(di_0.p2, BIN);    
    Serial.print(di_0.p3, BIN); 
    Serial.print(di_0.p4, BIN); 
    Serial.print(di_0.p5, BIN); 
    Serial.print(di_0.p6, BIN);
    Serial.print(di_0.p7, BIN);
    Serial.println("----------");   
  }
  
  if (managePcfInterrupt_1) {
    managePcfInterrupt_1 = false;
    Serial.println("PCF_1: ");
    di_1 = pcf8574_1.digitalReadAll();
    Serial.print(di_1.p0, BIN);    
    Serial.print(di_1.p1, BIN); 
    Serial.print(di_1.p2, BIN);    
    Serial.print(di_1.p3, BIN); 
    Serial.print(di_1.p5, BIN); 
    Serial.print(di_1.p6, BIN);
    Serial.print(di_1.p7, BIN);
    Serial.println("----------");    
  }
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

  pinMode(MP3_BUSY, INPUT_PULLUP);
  pinMode(HELP_BTN,INPUT_PULLUP);
  pinMode(PUCK_1,INPUT_PULLUP);  
  pinMode(PUCK_2,INPUT_PULLUP);  
  pinMode(PUCK_3,INPUT_PULLUP);  
  pinMode(PUCK_4,INPUT_PULLUP);  
  pinMode(LID_CONTACT,INPUT_PULLUP);  

  pcf8574_0.pinMode(P0, INPUT); 
  pcf8574_0.pinMode(P1, INPUT); 
  pcf8574_0.pinMode(P2, INPUT); 
  pcf8574_0.pinMode(P3, INPUT); 
  pcf8574_0.pinMode(P4, INPUT);
  pcf8574_0.pinMode(P5, INPUT);
  pcf8574_0.pinMode(P6, INPUT);
  pcf8574_0.pinMode(P7, INPUT);

  
  pcf8574_1.pinMode(P0, INPUT); 
  pcf8574_1.pinMode(P1, INPUT);
  pcf8574_1.pinMode(P2, INPUT); 
  pcf8574_1.pinMode(P3, INPUT); 
  pcf8574_1.pinMode(P4, OUTPUT,LOW); //LEDS
  pcf8574_1.pinMode(P5, INPUT); //Cog
  pcf8574_1.pinMode(P6, INPUT); //Cog
  pcf8574_1.pinMode(P7, INPUT); //Cog


  currentState = INIT;
  lastState = NONE;
  
  
   Serial.print(F("Version: "));
   Serial.println(VERSION);
   helpBtn = digitalRead(HELP_BTN);    
   
   player.begin();

  if (pcf8574_0.begin()){
    Serial.println(F("PCF_0_0K"));
    Serial.println("");
  } else {
    Serial.println(F("PCF_0_ERROR"));
    while(1);
  }

  if (pcf8574_1.begin()){
    Serial.println(F("PCF_1_0K"));
    pcf8574_1.digitalWrite(P4, HIGH);
    Serial.println("");
  } else {
    Serial.println(F("PCF_1_ERROR"));
    while(1);
  }
  
}

void loop(){

  mp3Busy = digitalRead(MP3_BUSY);
  

  switch (currentState) {

    case NONE:
      break;

    case INIT:
      state_init();
      break;

    case STATE_0:
      state_0();
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
void pcf0_InterruptionRoutine(){
  managePcfInterrupt_0 = true;
  
}

void pcf1_InterruptionRoutine(){
  managePcfInterrupt_1 = true;
}
