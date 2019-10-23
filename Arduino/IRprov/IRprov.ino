#include <IRremote.h>
#define dw digitalWrite
#define dr digitalRead
#define aw analogWrite
#define ar analogRead

#define remote 12

IRsend irsend;
int mode = 2; // 모터 조절할 때

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}
int i=0;
void loop() {
  // put your main code here, to run repeatedly:
 if (mode ==2){
    irsend.sendSony(0xa,remote);
    mode=0;
    delay(1000);
 }
}
