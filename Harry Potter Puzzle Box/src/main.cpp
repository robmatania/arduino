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
#define DEFAULT_VOLUME MAX_VOLUME
#define SOUND_ONOFF  1 // 0 = off, 1 = on
#define VERSION 0.1
#define EPROM_STATE_ADDRESS 0

#define adressPCF8574_0 0x20 
#define adressPCF8574_1 0x21
#define adressPCF8574_2 0x22 
#define adressPCF8574_3 0x23

void pcf0_InterruptionRoutine();
void pcf1_InterruptionRoutine();
void pcf2_InterruptionRoutine();
void pcf3_InterruptionRoutine();

PCF8574::DigitalInput di_0;
PCF8574::DigitalInput di_1;
PCF8574::DigitalInput di_2;
PCF8574::DigitalInput di_3;

bool managePcfInterrupt_0 = false;
bool managePcfInterrupt_1 = false;
bool managePcfInterrupt_2 = false;
bool managePcfInterrupt_3 = false;

PCF8574 pcf8574_0(adressPCF8574_0,3,pcf0_InterruptionRoutine);
PCF8574 pcf8574_1(adressPCF8574_1,2,pcf1_InterruptionRoutine);
PCF8574 pcf8574_2(adressPCF8574_2,18,pcf2_InterruptionRoutine);
PCF8574 pcf8574_3(adressPCF8574_3,19,pcf3_InterruptionRoutine);

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
bool played = false;

byte newPuckStates;
byte lastPuckStates = 0;
byte newCogStates;
byte lastCogStates = 0;

int lidState;

byte gemSequence = 0;
bool gemPresent = false;
byte gemState = 0;

byte symbolCombi = 0;
byte di_0_lastByte = 0;
byte combiTry = 0;
unsigned long startTime;
unsigned long symbolTimeout = 8 * 1000;
bool timerActive = false;

int spellState = 0;   
unsigned int last_di_23 = 0;

int spellLedsTable[10] = {SPELL_LED_1, SPELL_LED_2, SPELL_LED_3, SPELL_LED_4, SPELL_LED_5, SPELL_LED_6, SPELL_LED_7, SPELL_LED_8, SPELL_LED_9, SPELL_LED_10};


DY::Player player(&Serial2);

Servo myservo; 


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
    Serial.println("Start Playing");
  }
}
// -------------------------------------------------------------------------------------
void startMp3PlaySynchro(int index,int vol){
  if (SOUND_ONOFF == 1){
    digitalWrite(HELP_LED,HIGH);
    player.setVolume(vol); 
    player.playSpecified(index);
    Serial.print("Start Playing synchro");
    Serial.println(index);
    mp3Busy = 0;
    delay(1000);
    while(mp3Busy == 0)
      mp3Busy = readMp3State();
    Serial.println("End Playing synhro");
     digitalWrite(HELP_LED,LOW);
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
      startMp3Play(3,DEFAULT_VOLUME);
  }
  lastPuckStates = newPuckStates;

  return newPuckStates;
  }
//--------------------------------------------------------------------------------------
byte readCogStates(bool playSound){

  newCogStates =  di_1.p7  | di_1.p6 << 1 | di_1.p5 << 2;
  if (newCogStates < lastCogStates){  
    Serial.println(newCogStates);  
    if (playSound)
      startMp3Play(4,DEFAULT_VOLUME);
  }
  lastCogStates = newCogStates;

  return newCogStates;
}
// -------------------------------------------------------------------------------------
bool symbolCombinaison(bool playSound){
  byte di_0_byte = 0;

  di_0_byte = di_0.p0<<7 | di_0.p1<<6  | di_0.p2<<5 | di_0.p3<<4 | di_0.p4<<3 | di_0.p5<<2 | di_0.p6<<1| di_0.p7;
  if ((di_0_byte != 0) && (di_0_byte != 0xFF) && (di_0_byte != di_0_lastByte)){
    di_0_lastByte = di_0_byte;
    combiTry++;
    // Serial.print(di_0_byte);
    if (playSound)
      startMp3Play(5,DEFAULT_VOLUME);
    if (di_0.p4 == 0)
      symbolCombi = symbolCombi | B00001000; // Hat
    else if (di_0.p5 == 0)
      symbolCombi = symbolCombi | B00000100; // 9 3/4
    else if (di_0.p1 == 0)
      symbolCombi = symbolCombi | B00000010; // Cauldron
    else if (di_0.p7 == 0)
      symbolCombi = symbolCombi | B00000001; // Cup
    else {
      symbolCombi = 0;
      }
    }
    if (di_0_byte == 0xFF)
      return false; // No touch
    else return true;
  }
  
// -------------------------------------------------------------------------------------


 byte readGemState(bool playSound){

  byte newGemState = digitalRead(GEM_1);

  byte tmpGem = digitalRead(GEM_2);
  newGemState = newGemState | tmpGem << 1;
  tmpGem = digitalRead(GEM_3);
  newGemState = newGemState | tmpGem << 2;
  tmpGem = digitalRead(GEM_4);
  newGemState = newGemState | tmpGem << 3;
/*
  Serial.print("Gem State: ");
  Serial.println(gemState);
  Serial.print("Gem Sequence: ");
  Serial.println(gemSequence);
  delay(1000);
*/
  switch (newGemState) {

    case 15: // Gem Absent
      digitalWrite(GEMLED_R,LOW);
      digitalWrite(GEMLED_G,LOW);
      gemPresent = false;
      break;
    
    case 7: // Right
      if (gemPresent == false){
        gemPresent = true;
        if (gemSequence == 3){
          digitalWrite(GEMLED_R,LOW);
          digitalWrite(GEMLED_G,HIGH);
          gemSequence = 4;
        }
        else {
          digitalWrite(GEMLED_G,LOW);
          digitalWrite(GEMLED_R,HIGH);
          if (playSound)
            startMp3Play(6,DEFAULT_VOLUME);
          gemSequence = 0;
        }
      }
      break;
        

    case 11: // Down
      if (gemPresent == false){
        gemPresent = true;
        if (gemSequence == 1){
          digitalWrite(GEMLED_R,LOW);
          digitalWrite(GEMLED_G,HIGH);
          gemSequence = 2;
        }
        else{
          digitalWrite(GEMLED_G,LOW);
          digitalWrite(GEMLED_R,HIGH);
          if (playSound)
            startMp3Play(6,DEFAULT_VOLUME);
          gemSequence = 0;
        }
      }
      break;

    case 13: // Left
      if (gemPresent == false){
        gemPresent = true;
        if (gemSequence == 0){
          digitalWrite(GEMLED_R,LOW);
          digitalWrite(GEMLED_G,HIGH);
          gemSequence = 1;
        }
        else {
          digitalWrite(GEMLED_G,LOW);
          digitalWrite(GEMLED_R,HIGH);
          if (playSound)
            startMp3Play(6,DEFAULT_VOLUME);
          gemSequence = 0;
        }
      }
      break;

    case 14: // Up
      if (gemPresent == false){
        gemPresent = true;
        if (gemSequence == 2){
          digitalWrite(GEMLED_R,LOW);
          digitalWrite(GEMLED_G,HIGH);
          gemSequence = 3;
      }
      else {
        digitalWrite(GEMLED_G,LOW);
        digitalWrite(GEMLED_R,HIGH);
        if (playSound)
            startMp3Play(6,DEFAULT_VOLUME);
        gemSequence = 0;
      }
      }
      break;

      default:
        Serial.println("ERROR, Unknown Gem State");
        break;
      }
   
    return gemSequence;
  }

// -------------------------------------------------------------------------------------
void openLatch(int latchPin)
{
  Serial.println("OPEN LATCH");
  digitalWrite(latchPin, HIGH);
  delay(200);
  digitalWrite(latchPin, LOW);
}
// -------------------------------------------------------------------------------------
void closeAllLatches(void)
{
  Serial.println("Close all latches");
  digitalWrite(LATCH_2, LOW);
  digitalWrite(LATCH_3, LOW);
  digitalWrite(LATCH_4, LOW);
  digitalWrite(LATCH_5, LOW);
}
// -------------------------------------------------------------------------------------
void clearAllSpellLeds(void){
  digitalWrite(SPELL_LED_1,LOW);
  digitalWrite(SPELL_LED_2,LOW);
  digitalWrite(SPELL_LED_3,LOW);
  digitalWrite(SPELL_LED_4,LOW);
  digitalWrite(SPELL_LED_5,LOW);
  digitalWrite(SPELL_LED_6,LOW);
  digitalWrite(SPELL_LED_7,LOW);
  digitalWrite(SPELL_LED_8,LOW);
  digitalWrite(SPELL_LED_9,LOW);
  digitalWrite(SPELL_LED_10,LOW);
  //Serial.println("LEDS CLEARED");
  }
// -------------------------------------------------------------------------------------
void sequenceSpellLeds(int cnt){
  int i;
  clearAllSpellLeds();
  for (int counter=0;counter<cnt;counter++){
    for (i=0;i<10;i++){
      if (i>0)
        digitalWrite(spellLedsTable[i-1],LOW);
      digitalWrite(spellLedsTable[i],HIGH);
      delay(100);
    }
    digitalWrite(spellLedsTable[i-1],LOW);
  }
}

void state_init() {

// If entering the state, do initialization stuff
  if (currentState != lastState) {     
    Serial.print(F("Enter State: "));
    Serial.println("INIT");    

    lastState = currentState;


  if (helpBtn == 0) {
/*  ***************   RESET GAME **************/
    Serial.println("OPEN LATCHES");
    openLatch(LATCH_2);
    delay(1000);
    openLatch(LATCH_3);
    delay(1000);
    openLatch(LATCH_4);
    delay(1000);
    openLatch(LATCH_5);
  
    Serial.println("Reset EEPROM"); 
    EEPROM.write(EPROM_STATE_ADDRESS,0xFF);

  }

  savedState = EEPROM.read(EPROM_STATE_ADDRESS);

  Serial.print(F("Initial State: "));
  Serial.println(savedState); 

  if (savedState == 0xFF)
    savedState = STATE_0; // First time
  
  currentState = puzzleStates(savedState);

  /******************   TEMP FORCE STATE *****************/
  //currentState = STATE_4;
  /*******************************************************/
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

/********* If entering the state, do initialization stuff *********/
  if (currentState != lastState) {     
    EEPROM.write(EPROM_STATE_ADDRESS,currentState);   

    Serial.print(F("Enter State: "));
    Serial.println(0);
    lastState = currentState;

    startMp3Play(2,DEFAULT_VOLUME);
    
  
  }


/********** Perform state tasks *********/
// Check for state transitions
/***
**** Uncomment when LidState works ****
    mp3Busy = readMp3State();
    if ((lidState == 1) && (mp3Busy == 0))
      stopMp3Play();
***/
#ifdef DEBUG
Serial.print(F("mp3 State: "));
Serial.println(mp3Busy);
#endif

 helpBtn = digitalRead(HELP_BTN); 
 if (helpBtn ==0){
  startMp3PlaySynchro(10,DEFAULT_VOLUME);
  startMp3PlaySynchro(11,DEFAULT_VOLUME);
  startMp3PlaySynchro(12,DEFAULT_VOLUME);
  startMp3PlaySynchro(13,DEFAULT_VOLUME);
 }


  readPuckStates(true); // Just play sound when puck is inserted. No effect on state transition.

  byte cogStates = readCogStates(true);
  if (cogStates == 0) {
// All Cogs are in place.  Turn on cog Led and change state to open Trap_2
   // Serial.print("Cog States: ");
   // Serial.println(cogStates);
    pcf8574_1.digitalWrite(P4, LOW); // Turn on Cog Led
    delay(2000);
    openLatch(LATCH_2);
    currentState = STATE_1; // Change state
  }
  else
    pcf8574_1.digitalWrite(P4, HIGH);
  

  
/********** If leaving the state, do clean up stuff  *********/
 if (currentState != lastState) {         // If we are leaving the state, do clean up stuff
    
  }

}
// -------------------------------------------------------------------------------------
void state_1() {
  // Gem puzzle
  // Winning sequence is:  Left, Down, Up, RightGem
  // Winning states are: 13, 11, 14, 7
  // Bad sequence or timeout resets sequence and plays lost sound.
  // Led Green if new position is correct or Red of incorrect.
  // Turn led off if gem not present.
  // Winning sequence plays sound and opens TRAP_3

/********* If entering the state, do initialization stuff *********/
  if (currentState != lastState) {         
    EEPROM.write(EPROM_STATE_ADDRESS,currentState);   
    lastState = currentState;
    Serial.print(F("Enter State: "));
    Serial.println(1);

    gemSequence = 0; // Initialise Gem sequence
    played = false;
   
  }

/********** Perform state tasks *********/
  helpBtn = digitalRead(HELP_BTN); 
  if ((helpBtn ==0) && (!played)){
    Serial.println("Play 14");
    startMp3PlaySynchro(14,DEFAULT_VOLUME);
    played = true;
 }

  helpBtn = digitalRead(HELP_BTN); 
  if (helpBtn ==0){
    startMp3PlaySynchro(15,DEFAULT_VOLUME);
 }


readPuckStates(true); // Just play sound when puck is inserted. No effect on state transition.

// Check for state transitions
gemState = readGemState(true);
if (gemState == 4) { // Gem sequence completed. Open TRAP_3
  startMp3Play(5,DEFAULT_VOLUME);
  delay(2000);
 // digitalWrite(GEMLED_R,LOW); // Turn off Leds
 // digitalWrite(GEMLED_G,LOW); // Turn off Leds
  openLatch(LATCH_3);
  currentState = STATE_2; 
  startMp3PlaySynchro(16,DEFAULT_VOLUME);
}
/********** If leaving the state, do clean up stuff  *********/
 if (currentState != lastState) {         // If we are leaving the state, do clean up stuff
    
  }
}
// -------------------------------------------------------------------------------------
void state_2() {
  // Symbols Puzzle
  // Winning combination is: Hat, 8/34, Cauldron, Cup
  // Bad combination or timeout resets combination and plays lost sound.
  // Winning combination plays sound and opens TRAP_4

// If entering the state, do initialization stuff
  if (currentState != lastState) {    
    EEPROM.write(EPROM_STATE_ADDRESS,currentState);   
    lastState = currentState;
    Serial.print(F("Enter State: "));
    Serial.println(2);     
    di_0_lastByte = 0;
    combiTry = 0;
    timerActive = false;
    lastState = currentState;

    symbolCombi = 0; // Initialise combination
  }

// Perform state tasks
readPuckStates(true); // Just play sound when puck is inserted. No effect on state transition.

helpBtn = digitalRead(HELP_BTN); 
  if (helpBtn ==0){
    startMp3PlaySynchro(19,DEFAULT_VOLUME);
 }

//delay (500);
byte touched = symbolCombinaison(true);
if ((touched) && (timerActive))
  startTime = millis(); // Restart timer

/*
Serial.print("Try: ");
Serial.print(combiTry);
Serial.print("Combi:");
Serial.println(symbolCombi);
*/
if ((combiTry == 1) && (!timerActive)){ // First try
/* Start Timer Here */
  startTime = millis();
  timerActive = true;
}

if (symbolCombi == B0001111)
// Combination completed
{
  delay(1000);
  startMp3Play(4,DEFAULT_VOLUME);
  delay(2000);
  openLatch(LATCH_5);
  currentState = STATE_3; 
}
else {

  if ((timerActive) && ((combiTry >= 4) || ((millis() - startTime) > symbolTimeout))){ 
    Serial.println("Failed");
    stopMp3Play();
    startMp3Play(6,DEFAULT_VOLUME);
    symbolCombi = 0;
    combiTry = 0;
    if (touched == false)
      di_0_lastByte = 0; 
    timerActive = false;
  } 
}

// Check for state transitions

// If leaving the state, do clean up stuff
 if (currentState != lastState) {         // If we are leaving the state, do clean up stuff
    
  }
}
// -------------------------------------------------------------------------------------
void state_3() {
  // Dragon spell puzzle
  // Winning condition is when dragon tube is inserted un trap_5
  // Winning condition is true, play a sound and change to state_4 to activate spell detection.


// If entering the state, do initialization stuff
  if (currentState != lastState) {         
    EEPROM.write(EPROM_STATE_ADDRESS,currentState);   
    Serial.print(F("Enter State: "));
    Serial.println(3);
    lastState = currentState;
   
  }

// Perform state tasks
readPuckStates(true); // Just play sound when puck is inserted. No effect on state transition.

helpBtn = digitalRead(HELP_BTN); 
  if (helpBtn ==0){
    startMp3PlaySynchro(17,DEFAULT_VOLUME);
 }

int gt = analogRead(DRAGON_TUBE);
//Serial.println(gt);
delay(500);
if (gt > 15){
    digitalWrite(DRAGON_SPELL_LED,HIGH);
    currentState = STATE_4;
}



// Check for state transitions

// If leaving the state, do clean up stuff
 if (currentState != lastState) {         // If we are leaving the state, do clean up stuff
    
  }
}
// -------------------------------------------------------------------------------------
void state_4() {
  // Spell puzzle
  // trace spell with wand by passing detectors in sequence.
  // If sequence error, puzzle resets.
  // Led is lit when sensor detected. Previous led is turned off.
  // when spell is correctly traced, LATCH_5 is opened.

// If entering the state, do initialization stuff
  if (currentState != lastState) {    
    EEPROM.write(EPROM_STATE_ADDRESS,currentState);   
    Serial.print(F("Enter State: "));
    Serial.println(4);
    spellState = 1;     
    lastState = currentState;
    last_di_23 = 0;
  }

// Perform state tasks
readPuckStates(true); // Just play sound when puck is inserted. No effect on state transition.
unsigned int di_23 = di_2.p0 | di_2.p1<<1 | di_2.p2<<2 |di_2.p3<<3 | di_2.p4<<4 | di_2.p5<<5 | di_2.p6<<6 | di_2.p7<<7 | di_3.p0<<8 | di_3.p1<<9;   
helpBtn = digitalRead(HELP_BTN); 
  if (helpBtn ==0){
    startMp3PlaySynchro(20,DEFAULT_VOLUME);
 }
//Serial.print("DI_23: ");
//Serial.println(di_23,BIN);
delay (100);

if ((di_23 != 0B1111111111) && (last_di_23 != di_23)){
  Serial.print("DETECTION. SpellState= ");
  Serial.println(spellState);

  last_di_23 = di_23;
  switch (spellState) {
    case 1:
      if (di_3.p0 == 0){
        clearAllSpellLeds();
        digitalWrite(SPELL_LED_1,HIGH);
        spellState = 2;
      }
      else {
        clearAllSpellLeds();
        startMp3Play(8,DEFAULT_VOLUME);
        spellState = 1;
      }
    break;

    case 2:
      if (di_2.p0 == 0){
        clearAllSpellLeds();
        digitalWrite(SPELL_LED_2,HIGH);
        spellState = 3;
      }
      else {
        clearAllSpellLeds();
        startMp3Play(8,DEFAULT_VOLUME);
        spellState = 1;
      }
    break;

    case 3:
      if (di_2.p7 == 0){
        clearAllSpellLeds();
        digitalWrite(SPELL_LED_3,HIGH);
        spellState = 4;
      }
      else {
        clearAllSpellLeds();
        startMp3Play(8,DEFAULT_VOLUME);
        spellState = 1;
      }
    break;

    case 4:
      if (di_2.p6 == 0){
        clearAllSpellLeds();
        digitalWrite(SPELL_LED_4,HIGH);
        spellState = 5;
      }
      else {
        clearAllSpellLeds();
        startMp3Play(8,DEFAULT_VOLUME);
        spellState = 1;
      }
    break;

    case 5:
      if (di_2.p5 == 0){
        clearAllSpellLeds();
        digitalWrite(SPELL_LED_5,HIGH);
        spellState = 6;
      }
      else {
        clearAllSpellLeds();
        startMp3Play(8,DEFAULT_VOLUME);
        spellState = 1;
      }
    break;

   case 6:
      if (di_2.p4 == 0){
        clearAllSpellLeds();
        digitalWrite(SPELL_LED_6,HIGH);
        spellState = 7;
      }
      else {
        clearAllSpellLeds();
        startMp3Play(8,DEFAULT_VOLUME);
        spellState = 1;
      }
    break;    

    case 7:
      if (di_2.p1 == 0){
        clearAllSpellLeds();
        digitalWrite(SPELL_LED_7,HIGH);
        spellState = 8;
      }
      else {
        clearAllSpellLeds();
        startMp3Play(8,DEFAULT_VOLUME);
        spellState = 1;
      }
    break;
  
    case 8:
      if (di_2.p2 == 0){
        clearAllSpellLeds();
        digitalWrite(SPELL_LED_8,HIGH);
        spellState = 9;
      }
      else {
        clearAllSpellLeds();
        startMp3Play(8,DEFAULT_VOLUME);
        spellState = 1;
      }
    break;

   case 9:
      if (di_3.p0 == 0){
        clearAllSpellLeds();
        digitalWrite(SPELL_LED_1,HIGH);
        spellState = 10;
      }
      else {
        clearAllSpellLeds();
        startMp3Play(8,DEFAULT_VOLUME);
        spellState = 1;
      }
    break;

   case 10:
      if (di_2.p3 == 0){
        clearAllSpellLeds();
        digitalWrite(SPELL_LED_9,HIGH);
        spellState = 11;
      }
      else {
        clearAllSpellLeds();
        startMp3Play(8,DEFAULT_VOLUME);
        spellState = 1;
      }
    break;

   case 11:
      if (di_3.p1 == 0){
        clearAllSpellLeds();
        digitalWrite(SPELL_LED_10,HIGH);
        spellState = 12;
      }
      else {
        clearAllSpellLeds();
        startMp3Play(8,DEFAULT_VOLUME);
        spellState = 1;
      }
      break;

    case 12:  
    if (di_2.p5 == 0){
        startMp3Play(9,DEFAULT_VOLUME);
        clearAllSpellLeds();
        digitalWrite(SPELL_LED_5,HIGH);
        sequenceSpellLeds(2);
        digitalWrite(SPELL_LED_5,LOW);

        delay(1000);
        openLatch(LATCH_4);
        spellState = 13;
        currentState = STATE_5;
      }
      else {
        clearAllSpellLeds();
        startMp3Play(8,DEFAULT_VOLUME);
        spellState = 1;
      }
      break;

    default:
      break;
  }
}
// Check for state transitions

// If leaving the state, do clean up stuff
 if (currentState != lastState) {         
    
  }
}
// -------------------------------------------------------------------------------------
void state_5() {
  // Puck puzzle
  // All pucks should have been obtained.
  // When all 4 pucks are in place, play end of game sound, open chamber.

// If entering the state, do initialization stuff
  if (currentState != lastState) {    
    EEPROM.write(EPROM_STATE_ADDRESS,currentState);   
    Serial.print(F("Enter State: "));
    Serial.println(5);    
    lastState = currentState;
   
  }

// Perform state tasks
byte pucks = readPuckStates(true); // Just play sound when puck is inserted. No effect on state transition.

helpBtn = digitalRead(HELP_BTN); 
  if (helpBtn ==0){
    startMp3PlaySynchro(18,DEFAULT_VOLUME);
 }

// Check for state transitions
//Serial.print("puck status: ");
//Serial.print(pucks);
//delay(1000);
if (pucks == 0){
  // Game solved
  // Play winning sound and OPEN CHAMBER
  // Go to STATE_6 idle state
  startMp3Play(7,DEFAULT_VOLUME);
  delay(2000);
  myservo.write(FWD_SPEED);
  delay(1500);
  myservo.write(90);

  currentState = STATE_6;
}
// If leaving the state, do clean up stuff
 if (currentState != lastState) {        

  }
}
// -------------------------------------------------------------------------------------
void state_6() {
// If entering the state, do initialization stuff
  if (currentState != lastState) {
    EEPROM.write(EPROM_STATE_ADDRESS,currentState);   
    Serial.print(F("Enter State: "));
    Serial.println(6);             
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
  pinMode(GEM_1,INPUT_PULLUP);
  pinMode(GEM_2,INPUT_PULLUP);
  pinMode(GEM_3,INPUT_PULLUP);
  pinMode(GEM_4,INPUT_PULLUP);

  pinMode(DRAGON_TUBE, INPUT);
  
  pinMode(DRAGON_SPELL_LED,OUTPUT);
  pinMode(GEMLED_G,OUTPUT);
  pinMode(GEMLED_R,OUTPUT);

  pinMode(LATCH_2,OUTPUT);
  pinMode(LATCH_3,OUTPUT);
  pinMode(LATCH_4,OUTPUT);
  pinMode(LATCH_5,OUTPUT);


  pinMode(SPELL_LED_1,OUTPUT);
  pinMode(SPELL_LED_2,OUTPUT);
  pinMode(SPELL_LED_3,OUTPUT);
  pinMode(SPELL_LED_4,OUTPUT);
  pinMode(SPELL_LED_5,OUTPUT);
  pinMode(SPELL_LED_6,OUTPUT);
  pinMode(SPELL_LED_7,OUTPUT);
  pinMode(SPELL_LED_8,OUTPUT);
  pinMode(SPELL_LED_9,OUTPUT);
  pinMode(SPELL_LED_10,OUTPUT);
  pinMode(HELP_LED,OUTPUT);

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

  pcf8574_2.pinMode(P0, INPUT); 
  pcf8574_2.pinMode(P1, INPUT); 
  pcf8574_2.pinMode(P2, INPUT); 
  pcf8574_2.pinMode(P3, INPUT); 
  pcf8574_2.pinMode(P4, INPUT);
  pcf8574_2.pinMode(P5, INPUT);
  pcf8574_2.pinMode(P6, INPUT);
  pcf8574_2.pinMode(P7, INPUT);

  pcf8574_3.pinMode(P0, INPUT); 
  pcf8574_3.pinMode(P1, INPUT); 
  pcf8574_3.pinMode(P2, INPUT); 
  pcf8574_3.pinMode(P3, INPUT); 
  pcf8574_3.pinMode(P4, INPUT);
  pcf8574_3.pinMode(P5, INPUT);
  pcf8574_3.pinMode(P6, INPUT);
  pcf8574_3.pinMode(P7, INPUT);

  currentState = INIT;
  lastState = NONE;

  closeAllLatches();
  
  myservo.attach(7);
  myservo.write(90); // Servo is stationary.
  
   Serial.print(F("Version: "));
   Serial.println(VERSION);
   helpBtn = digitalRead(HELP_BTN); 
   
   player.begin();

  if (pcf8574_0.begin()){
    Serial.println(F("PCF_0_0K"));
    Serial.println("");
    di_0 = pcf8574_0.digitalReadAll(); // Initialise di_0
  } else {
    Serial.println(F("PCF_0_ERROR"));
    while(1);
  }

  if (pcf8574_1.begin()){
    Serial.println(F("PCF_1_0K"));
    di_1 = pcf8574_1.digitalReadAll();  // Initialise di_1
    pcf8574_1.digitalWrite(P4, HIGH); // Turn off Cog Led
    Serial.println("");
  } else {
    Serial.println(F("PCF_1_ERROR"));
    while(1);
  }
  
if (pcf8574_2.begin()){
    Serial.println(F("PCF_2_0K"));
    Serial.println("");
    di_2 = pcf8574_2.digitalReadAll(); // Initialise di_2
  } else {
    Serial.println(F("PCF_2_ERROR"));
    while(1);
  }

if (pcf8574_3.begin()){
    Serial.println(F("PCF_3_0K"));
    Serial.println("");
    di_3 = pcf8574_3.digitalReadAll(); // Initialise di_3
  } else {
    Serial.println(F("PCF_3_ERROR"));
    while(1);
  }

  digitalWrite(DRAGON_SPELL_LED,LOW);

 
/*
   Serial.print(di_0.p0, BIN);    
   Serial.print(di_0.p1, BIN); 
   Serial.print(di_0.p2, BIN);    
   Serial.print(di_0.p3, BIN); 
   Serial.print(di_0.p4, BIN); 
   Serial.print(di_0.p5, BIN); 
   Serial.print(di_0.p6, BIN);
   Serial.print(di_0.p7, BIN);
   Serial.println("----------");   

*/

}

void loop(){

  mp3Busy = digitalRead(MP3_BUSY);
  

  if (managePcfInterrupt_0) {
    managePcfInterrupt_0 = false;
  //  Serial.println("PCF_0: ");
    di_0 = pcf8574_0.digitalReadAll();
  }
  
  if (managePcfInterrupt_1) {
    managePcfInterrupt_1 = false;
  //  Serial.println("PCF_1: ");
    di_1 = pcf8574_1.digitalReadAll();
  }

if (managePcfInterrupt_2) {
    managePcfInterrupt_2 = false;
    Serial.println("PCF_2: ");
    di_2 = pcf8574_2.digitalReadAll();
  }

  if (managePcfInterrupt_3) {
    managePcfInterrupt_3 = false;
    Serial.println("PCF_3: ");
    di_3 = pcf8574_3.digitalReadAll();
  }

  

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
  
    case STATE_4:
      state_4();
      break;
      
    case STATE_5:
      state_5();
      break;

    case STATE_6:
      state_6();
      break;

    default:
      break;
  }
 // Read key to manually move servo
if (Serial.available() > 0){
  byte inChar = Serial.read();
  Serial.println(inChar);
  if (inChar == 32)
    myservo.write(90);
  else if (inChar == 49){
    Serial.print("MOVE");
   myservo.write(FWD_SPEED);
   delay(100);
  }
  else if (inChar == 50){
   myservo.write(FWD_SPEED);
   delay(200);
  }
  else if (inChar == 51){
   myservo.write(FWD_SPEED);
   delay(500);
  }
  else if (inChar == 52){
   myservo.write(BCK_SPEED);
   delay(100);
  }
  else if (inChar == 53){
   myservo.write(BCK_SPEED);
   delay(200);
  }
  else if (inChar == 54){
   myservo.write(BCK_SPEED);
   delay(500);
  }
  myservo.write(90);
}


}
void pcf0_InterruptionRoutine(){
  managePcfInterrupt_0 = true;
}
void pcf1_InterruptionRoutine(){
  managePcfInterrupt_1 = true;
}
void pcf2_InterruptionRoutine(){
  managePcfInterrupt_2 = true;
}
void pcf3_InterruptionRoutine(){
  managePcfInterrupt_3 = true;
}