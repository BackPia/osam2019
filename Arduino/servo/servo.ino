#include <Servo.h>

#define dw digitalWrite
#define dr digitalRead
#define aw analogWrite
#define ar analogRead
#include <Servo.h>
Servo servo;
int buttonPin = 8;
  int sw = 0;

void setup() {
  Serial.begin(9600);
 
  servo.attach(11);
  servo.write(180);
   pinMode(buttonPin, INPUT_PULLUP);
}
void loop() {

  if (digitalRead(buttonPin) == LOW){
    sw++;
    delay(1000); 
    if (sw>16){
      sw=0;
    }
  }
  servo.write(10*sw);
  
  
}
