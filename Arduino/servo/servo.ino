#include <Servo.h>

#define dw digitalWrite
#define dr digitalRead
#define aw analogWrite
#define ar analogRead
#include <Servo.h>
Servo servo;

void setup() {
  Serial.begin(9600);
  pinMode(2,INPUT);
  servo.attach(11);
  servo.write(180);
}
void loop() {
 /* int al;
  al=ar(0)/2;
  Serial.println(al);
  servo.write(al); 
  delay(50);*/

  while(dr(2)!=HIGH);
  servo.write(180);
  delay(200);
  while(dr(2)!=HIGH);
  servo.write(70);
  delay(200);
  
}
