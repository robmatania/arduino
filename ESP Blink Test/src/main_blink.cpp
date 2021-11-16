#include <Arduino.h>
#include <Ticker.h>
#define ONBOARD_LED  23

// Comment
int buttonState = 0;
bool ledState;
bool mib = true;
bool mailDetectedFlag = true;




void ledOnCallback();
void ledOffCallback();

void IRAM_ATTR buttonMailReset() {
  mib = false;
}
void IRAM_ATTR mailDetected() {
  if (mailDetectedFlag == false) {
  mib = true;
  mailDetectedFlag = true;
  }
}

Ticker timer0(ledOnCallback, 100,1); 
Ticker timer1(ledOffCallback, 1000,1); 

void setup() {
  // put your setup code here, to run once:
   pinMode(ONBOARD_LED,OUTPUT);
   pinMode(21,OUTPUT);
   Serial.begin(9600);
   pinMode(32, INPUT);
   pinMode(22, INPUT);

   
   ledState = false;
   attachInterrupt(22, buttonMailReset, RISING);
   attachInterrupt(32, mailDetected, FALLING);

   //timer0.start();
   
}

void loop() {
  
  /*
  buttonState = digitalRead(22);
  Serial.println(buttonState);
  delay(100);
*/



  if (mailDetectedFlag == true) {
   timer0.start();
   mailDetectedFlag = false;
  }
  timer0.update();
  timer1.update();
 
}
void ledOnCallback() {
  digitalWrite(ONBOARD_LED, LOW);
  digitalWrite(21, LOW);
  if (mib == true)
     timer1.start();
  }

  void ledOffCallback() {
  if (mib == true)
    digitalWrite(ONBOARD_LED, HIGH);
    digitalWrite(21, HIGH);
  timer0.start();
  }