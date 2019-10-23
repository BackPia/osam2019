#include <SimpleTimer.h>

#define minTime 1
#define dw digitalWrite
#define dr digitalRead
#define aw analogWrite
#define ar analogRead

const byte dec_digits[] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 
0b01101101, 0b01111100, 0b00000111, 0b01111111, 0b01100111 };
int data=2;
int clk=3;
int lat=4;
int num=0;
int seq=0,seqcnt=0,dot=0;
SimpleTimer timer(minTime);

void setup() {
  // put your setup code here, to run once:
  pinMode(data,OUTPUT);
  pinMode(lat,OUTPUT);
  pinMode(clk,OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(timer.isReady()){
    timer.reset();
    ms();
    
  }
}

void ms() {
  byte dt;
  int n1=0,seq_c=(1<<seq);
  if(seqcnt>=3){
    dw(lat,HIGH);
    shiftOut(data,clk,MSBFIRST,seq_c);
    switch(seq){
     case 0: n1=num/1000%10; break;
     case 1: n1=num/100%10; break;
     case 2: n1=num/10%10; break;
     case 3: n1=num%10; break;
    }
   dt=dec_digits[n1];
   if(dot-1==3-seq)dt|=0x80;
   shiftOut(data,clk,MSBFIRST,~dt);
   
    seqcnt=0;
    seq=seq+1>=4?0:seq+1;
    num++;
    if(num>9999)num=0;
    dw(lat,LOW);
    
  }
  seqcnt++;
}
