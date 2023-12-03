#include <Arduino.h>

//int hallSensorPin = A0;     
int hallSensorPin = 2;     
int fetOpPin = 3;
int ledPin =  13;    
int state = 0; 
long sensorValue;
bool sVal;

void setup() {
  Serial.begin(9600);
  
  pinMode(ledPin, OUTPUT);      
  pinMode(hallSensorPin, INPUT_PULLUP);     
  pinMode(fetOpPin,OUTPUT);

for (int i=0; i<3; i++)
  {
  digitalWrite(fetOpPin, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(fetOpPin, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
  }
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
    digitalWrite(fetOpPin, LOW); 
  } 
  else {
    digitalWrite(ledPin, LOW); 
    digitalWrite(fetOpPin, HIGH); 
  }
 

/*
 digitalWrite(fetOpPin, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(fetOpPin, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
  */
}