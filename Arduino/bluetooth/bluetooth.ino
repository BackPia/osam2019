#include <SoftwareSerial.h>
SoftwareSerial btSerial(7,8);       //bluetooth

int bt_time = 0;
char bt_temp;

void setup() 
{
  Serial.begin(9600);
  btSerial.begin(9600);
}

void loop()
{
  if(Serial.available()) //시리얼 대신 센서값으로 변경
    btSerial.write(Serial.read()); //100값을 주면 물이 다찼음을 알림
    
  if(btSerial.available()){
    bt_temp = btSerial.read();
    
    switch(bt_temp) {
      //차갑게
      case 'A':
        bt_time=btSerial.parseInt();
        if(btSerial.read()=='.')break;
        break;
      //시원
      case 'B':
        bt_time=btSerial.parseInt();
        if(btSerial.read()=='.')break;
        break;
      //따뜻
      case 'C':
        bt_time=btSerial.parseInt();
        if(btSerial.read()=='.')break;
        break;
      //뜨겁
      case 'D':
        bt_time=btSerial.parseInt();
        if(btSerial.read()=='.')break;
        break;
      }
      Serial.print(bt_temp);
      Serial.print("|");
      Serial.println(bt_time);
    }
  }
