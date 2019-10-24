#include <IRremote.h>
#define dw digitalWrite
#define dr digitalRead
#define aw analogWrite
#define ar analogRead
#include <Servo.h>
Servo servo1,servo2;
#define recv 7

IRrecv irrecv(recv);
decode_results res;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(recv,INPUT);
  irrecv.enableIRIn();
  servo1.attach(11);
  servo2.attach(10);
  servo1.write(45);
  servo2.write(40);
}

void loop() {
  // put your main code here, to run repeatedly:

  if(irrecv.decode(&res)){
    if (res.value <=0xf && res.value>=0x0){Serial.println(res.value);
        Serial.println(res.value,HEX);
#define D 100
        switch(res.value&0b0011){ //냉수
          case 0b00: servo1.write(100); delay(D); servo1.write(70); delay(D); servo1.write(45); break;
          case 0b01: servo1.write(70); delay(D); servo1.write(100); delay(D); servo1.write(130); break;
          //case 0b10: servo1.write(80); break;
          //case 0b11: servo1.write(40); break;
        }
        switch((res.value&0b1100)>>2){  //온수
          case 0b00: servo2.write(100); delay(D); servo2.write(70); delay(D); servo2.write(45); break;
          case 0b01: servo2.write(70); delay(D); servo2.write(100); delay(D); servo2.write(130); break;
          //case 0b10: servo2.write(80); break;
          //case 0b11: servo2.write(40); break;
        }
        
    }
    
    irrecv.resume();
  }
}
