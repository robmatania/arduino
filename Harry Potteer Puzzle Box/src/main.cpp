#include <Arduino.h>

//int hallSensorPin = A0;     
int hallSensorPin = 2;     

int ledPin =  13;    
int state = 0; 
long sensorValue;
bool sVal;

void setup() {
  Serial.begin(9600);
  
  pinMode(ledPin, OUTPUT);      
  pinMode(hallSensorPin, INPUT_PULLUP);     
}

void loop(){
  
  /**
   sensorValue = analogRead(hallSensorPin);

   delay (1);
  //Serial.println(sensorValue);

  if (sensorValue > 600) {        
    digitalWrite(ledPin, HIGH);  
  } 
  else {
    digitalWrite(ledPin, LOW); 
  }
  **/
 sVal = digitalRead(hallSensorPin);
 if (sVal) {        
    digitalWrite(ledPin, HIGH);  
  } 
  else {
    digitalWrite(ledPin, LOW); 
  }
}