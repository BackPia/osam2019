#include <IRremote.h>
#define dw digitalWrite
#define dr digitalRead
#define aw analogWrite
#define ar analogRead

#define recv 7

IRrecv irrecv(recv);
decode_results res;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  irrecv.enableIRIn();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(irrecv.decode(&res)){
    if (res.value < 0xaff && res.value >0xa00){
        Serial.println(res.value,HEX);
    }
    irrecv.resume();
  }
}
