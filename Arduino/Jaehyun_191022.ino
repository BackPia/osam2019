#include "LedControl.h"
#include "DHT.h"
#define DHTPIN 7     // DHT11이 연결된 핀

#define DHTTYPE DHT11   // DHT 11, DHT시리즈중 11을 선택합니다.

DHT dht(DHTPIN, DHTTYPE);


 
LedControl lc = LedControl(12,11,10,1);
 
unsigned long delaytime = 100;
 
void setup() {
  
 
  lc.shutdown(0,false);       // 첫 번째 드라이버의 절전모드를 해제
 
  lc.setIntensity(0,4);       // 밝기를 중간으로 설정
 
  lc.clearDisplay(0);         // 디스플레이 초기화
  Serial.begin(9600);   
  dht.begin();
}
 
 
byte wave_Medium[7] = {B00000000,B01100000,B11110000, B11111001,B11111111};




void high(){
byte temp;
lc.setRow(0,0,wave_Medium[0]);
lc.setRow(0,1,wave_Medium[1]);
lc.setRow(0,2,wave_Medium[1]);
lc.setRow(0,3,wave_Medium[2]);
lc.setRow(0,4,wave_Medium[2]);
lc.setRow(0,5,wave_Medium[3]);
lc.setRow(0,6,wave_Medium[3]);
lc.setRow(0,7,wave_Medium[4]);  
delay(300);
for(int i=0;i<5;i++){
temp=wave_Medium[i]&0b00000001;
temp=temp<<7;
wave_Medium[i]=wave_Medium[i]>>1|temp;}
}

void medium(){
byte temp;
lc.setRow(0,0,wave_Medium[0]);
lc.setRow(0,1,wave_Medium[0]);
lc.setRow(0,2,wave_Medium[1]);
lc.setRow(0,3,wave_Medium[2]);
lc.setRow(0,4,wave_Medium[3]);
lc.setRow(0,5,wave_Medium[4]);
lc.setRow(0,6,wave_Medium[4]);
lc.setRow(0,7,wave_Medium[4]);  
delay(300);
for(int i=0;i<7;i++){
temp=wave_Medium[i]&0b00000001;
temp=temp<<7;
wave_Medium[i]=wave_Medium[i]>>1|temp;}
}

void low(){
byte temp;

lc.setRow(0,0,wave_Medium[0]);
lc.setRow(0,1,wave_Medium[0]);
lc.setRow(0,2,wave_Medium[0]);
lc.setRow(0,3,wave_Medium[2]);
lc.setRow(0,4,wave_Medium[4]);
lc.setRow(0,5,wave_Medium[4]);
lc.setRow(0,6,wave_Medium[4]);
lc.setRow(0,7,wave_Medium[4]);  
delay(300);
for(int i=0;i<7;i++){
temp=wave_Medium[i]&0b00000001;
temp=temp<<7;
wave_Medium[i]=wave_Medium[i]>>1|temp;}
}



void loop(){

byte Small_right[5] = {B00000000,B00000110, B00001111,B10011111,B11111111};

int val = analogRead(A0);
unsigned char t = dht.readTemperature()-5 ;





if (val >630){
  high();
}
else if (val > 550){
  medium();
}
else{
  low();
}

Serial.println(t);
}
