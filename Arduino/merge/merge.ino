#include "LedControl.h"
#include "DHT.h"
#include <SimpleTimer.h>

#define minTime 1
#define dw digitalWrite
#define dr digitalRead
#define aw analogWrite
#define ar analogRead

const byte dec_digits[] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101, 0b01111100, 0b00000111, 0b01111111, 0b01100111 };
byte wave_Medium[7] = {B00000000,B01100000,B11110000, B11111001,B11111111};
#define seg_data  2
#define seg_clk   3
#define seg_lat   4
#define dht_PIN   9
#define dot_cs    10
#define dot_clk   11
#define dot_din   12

int seg_num=0,seg_dot=0; //4*7segment
int dot_mode=0;          //8*8dot
int temp=0,waterlevel=0;
SimpleTimer timer(minTime);
LedControl lc = LedControl(dot_din,dot_clk,dot_cs,1);
DHT dht(dht_PIN,DHT11);

byte shiftbyte(byte b){
  byte temp=b&0b00000001;
  temp=temp<<7;
  b=b>>1|temp;
  return b;
}

void setup() {
  Serial.begin(9600);

  // 8*8 dot init
  lc.shutdown(0,false);       // 첫 번째 드라이버의 절전모드를 해제
  lc.setIntensity(0,4);       // 밝기를 중간으로 설정
  lc.clearDisplay(0);         // 디스플레이 초기화
  dht.begin();

  // 4*7 segment init
  pinMode(seg_data,OUTPUT);
  pinMode(seg_lat,OUTPUT);
  pinMode(seg_clk,OUTPUT);
  
}

void loop() {
  if(timer.isReady()){
    timer.reset();
    temp_ms();
    seg_ms();
    dot_ms();
    
    dot_mode=1;//==============================
  }
}
void seg_ms() {
  static int seg_seq=0,seg_seqcnt=0;
  byte dt;
  int n1=0,seq_c=(1<<seg_seq);
  
  seg_seqcnt++;
  if(seg_seqcnt>=3){
    dw(seg_lat,HIGH);
    shiftOut(seg_data,seg_clk,MSBFIRST,seq_c);
    switch(seg_seq){
     case 0: n1=seg_num/1000%10; break;
     case 1: n1=seg_num/100%10; break;
     case 2: n1=seg_num/10%10; break;
     case 3: n1=seg_num%10; break;
    }
   dt=dec_digits[n1];
   if(seg_dot-1==3-seg_seq)dt|=0x80;
   shiftOut(seg_data,seg_clk,MSBFIRST,~dt);
   
    seg_seqcnt=0;
    seg_seq=seg_seq+1>=4?0:seg_seq+1;
    seg_num++;
    if(seg_num>9999)seg_num=0;
    dw(seg_lat,LOW);
  }
}
void dot_ms(){
  static int dot_seq=0,dot_seqcnt=0;
  dot_seqcnt++;
  if(dot_mode==1){
    if(dot_seqcnt>=300){
      dot_seqcnt=0;
      if(waterlevel<30){
        //low
        lc.setRow(0,0,wave_Medium[0]);
        lc.setRow(0,1,wave_Medium[0]);
        lc.setRow(0,2,wave_Medium[0]);
        lc.setRow(0,3,wave_Medium[2]);
        lc.setRow(0,4,wave_Medium[4]);
        lc.setRow(0,5,wave_Medium[4]);
        lc.setRow(0,6,wave_Medium[4]);
        lc.setRow(0,7,wave_Medium[4]);
      }else if(waterlevel<60){
        //mid
        lc.setRow(0,0,wave_Medium[0]);
        lc.setRow(0,1,wave_Medium[0]);
        lc.setRow(0,2,wave_Medium[1]);
        lc.setRow(0,3,wave_Medium[2]);
        lc.setRow(0,4,wave_Medium[3]);
        lc.setRow(0,5,wave_Medium[4]);
        lc.setRow(0,6,wave_Medium[4]);
        lc.setRow(0,7,wave_Medium[4]);
      }else {
        //high
        lc.setRow(0,0,wave_Medium[0]);
        lc.setRow(0,1,wave_Medium[1]);
        lc.setRow(0,2,wave_Medium[1]);
        lc.setRow(0,3,wave_Medium[2]);
        lc.setRow(0,4,wave_Medium[2]);
        lc.setRow(0,5,wave_Medium[3]);
        lc.setRow(0,6,wave_Medium[3]);
        lc.setRow(0,7,wave_Medium[4]);
      }
      
      for(int i=0;i<7;i++){
          wave_Medium[i]=shiftbyte(wave_Medium[i]);
        }
    }
  }
}
void temp_ms(){
  static int temp_seqcnt=0;
  temp_seqcnt++;
  if(temp_seqcnt>=100){
    temp_seqcnt=0;
    temp=dht.readTemperature()-5 ;
    waterlevel=analogRead(A0);
    if(waterlevel<350){
      waterlevel=0;
    } else {
      waterlevel=map(waterlevel,300,700,1,100);
    }
  }
}
