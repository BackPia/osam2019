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
  irrecv.enableIRIn();
  servo1.attach(10);
  servo2.attach(11);
}

void loop() {
  // put your main code here, to run repeatedly:

  if(irrecv.decode(&res)){
    if (res.value <=0xa && res.value>=0x0){Serial.println(res.value);
        Serial.println(res.value,HEX);
        
        if (res.value & 0x1){
          servo1.write(160);
        }
        else if (res.value & 0x2){
          servo1.write(120);//냉 2모드
        }
        else {
          servo1.write(40);//냉 0모드
        }
        
        if (res.value &0x4){
         servo2.write(160);// 온 1모드
        }
        else if (res.value & 0x8){
          servo2.write(120);//냉 2모드
        }
        else {
          servo2.write(40);//냉 0모드
        }
        
    }
    
    irrecv.resume();
  }
}
