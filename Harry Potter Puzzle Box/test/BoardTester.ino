#include <HPPuzzleBox.h>
#include "DYPlayerArduino.h"
#include "PinChangeInterrupt.h"
#include <PCF8574.h>


#define adresseDuModulePCF8574_0 0x22           // Adresse i2c du PCF8574 (attention : dépend de la configuration des broches A2/A1/A0 de cette puce)
#define adresseDuModulePCF8574_1 0x23 
void pcf0_InterruptionRoutine();
void pcf1_InterruptionRoutine();
PCF8574 pcf8574_0(adresseDuModulePCF8574_0,18,pcf0_InterruptionRoutine);
PCF8574 pcf8574_1(adresseDuModulePCF8574_1,19,pcf1_InterruptionRoutine);


DY::Player player(&Serial2);


void setup() {

Serial.begin(9600);

pinMode(GEMLED_G,OUTPUT);
pinMode(GEMLED_R,OUTPUT);

pinMode(GEM_1,INPUT_PULLUP);
pinMode(GEM_2,INPUT_PULLUP);
pinMode(GEM_3,INPUT_PULLUP);
pinMode(GEM_4,INPUT_PULLUP);

pinMode(PUCK_1, INPUT_PULLUP);
pinMode(PUCK_2, INPUT_PULLUP);
pinMode(PUCK_3, INPUT_PULLUP);
pinMode(PUCK_4, INPUT_PULLUP);

pinMode(HELP_BTN,INPUT_PULLUP);
pinMode(HELP_LED,OUTPUT);

pinMode(LATCH_2,OUTPUT);
pinMode(LATCH_3,OUTPUT);
pinMode(LATCH_4,OUTPUT);
pinMode(LATCH_5,OUTPUT);

pinMode(MP3_BUSY, INPUT_PULLUP);

attachPCINT(digitalPinToPCINT(MP3_BUSY), mp3BusyInterruptHandler, CHANGE);

delay(1000);

/*
player.begin();
player.setVolume(30); // 50% Volume
Serial.println("Start Playing");
player.playSpecified(1);
*/

}

void loop() {
  /*
  digitalWrite(GEMLED_R,HIGH);
  delay(1000);
  digitalWrite(GEMLED_R,LOW);
  digitalWrite(GEMLED_G,HIGH);
  delay(1000);
  digitalWrite(GEMLED_G,LOW);
  delay(1000);
  */

  /*
  int g1 = digitalRead(PUCK_1);
  int g2 = digitalRead(PUCK_2);
  int g3 = digitalRead(PUCK_3);
  int g4 = digitalRead(PUCK_4);

Serial.print(g1);
Serial.print(" ");
Serial.print(g2);
Serial.print(" ");
Serial.print(g3);
Serial.print(" ");
Serial.println(g4);
delay(1000);

*/
/*
 int h1 = digitalRead(HELP_BTN);
  Serial.println(h1);
 if (h1 == 0)
  digitalWrite(HELP_LED, true);
 else
  digitalWrite(HELP_LED, false);
*/

/*
if (Serial.available() > 0)
{
  byte inChar = Serial.read();
   //Serial.println(inChar);
   if (inChar == 49)
    openLatch(LATCH_2);
   else if (inChar == 50)
    openLatch(LATCH_3);
  else if (inChar == 51)
    openLatch(LATCH_4);
  else if (inChar == 52)
    openLatch(LATCH_5);

}
*/



}

void openLatch(int latchPin)
{
  digitalWrite(latchPin, HIGH);
  delay(200);
  digitalWrite(latchPin, LOW);
}

void mp3BusyInterruptHandler(void){
  int mp3Busy = digitalRead(MP3_BUSY);
 digitalWrite(GEMLED_R, !mp3Busy);
}
