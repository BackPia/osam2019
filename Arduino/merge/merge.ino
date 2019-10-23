#include "LedControl.h"
#include <SimpleTimer.h>
#include <IRremote.h>
#include <SoftwareSerial.h>

#define minTime 1
#define dw digitalWrite
#define dr digitalRead
#define aw analogWrite
#define ar analogRead

const byte dec_digits[] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101, 0b01111100, 0b00000111, 0b01111111, 0b01100111 };
byte wave_Medium[7] = {B00000000,B01100000,B11110000, B11111001,B11111111};
#define seg_data  4   //orange
#define seg_clk   5   //yellow
#define seg_lat   6   //blue
#define dht_PIN   9   //orange
#define dot_din   10  //orange
#define dot_cs    11  //yellow
#define dot_clk   12  //green

int seg_num=0,seg_dot=0,seg_mode=0; //4*7segment
int dot_mode=0;                     //8*8dot
int temp=0,waterlevel=0,temp_on=1;//temp,water senser
int ir_trg=0,ir_data=0;             //ir control
int bt_time=0,bt_trg=0;char bt_temp;//bluetooth
SoftwareSerial btSerial(7,8);       //bluetooth
SimpleTimer timer(minTime);         // 1ms timer interupt
LedControl lc = LedControl(dot_din,dot_clk,dot_cs,1); //8*8 dotmatix
IRsend irsend;                      // IRremote

byte shiftbyte(byte b){             // dot's byte shift
  byte temp=b&0b00000001;
  temp=temp<<7;
  b=b>>1|temp;
  return b;
}
double GetTemperature(int v)        //analog temp val -> double temp val
{
 double Temp;
 Temp = log(10000.0 / (1024.0 / v - 1)); 
 Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp))* Temp);
 Temp = Temp - 273.15; // Convert Kelvin to Celcius
// Temp = (Temp * 9.0) / 5.0 + 32.0; // Convert Celcius to Fahrenheit
 return Temp;
}

void setup() {
  Serial.begin(9600);         //시리얼 모니터 on
  btSerial.begin(9600);       //블루투스 on

  // 8*8 dot init
  lc.shutdown(0,false);       // 첫 번째 드라이버의 절전모드를 해제
  lc.setIntensity(0,4);       // 밝기를 중간으로 설정
  lc.clearDisplay(0);         // 디스플레이 초기화
  lc.setRow(0,0,0b0);
  lc.setRow(0,1,0b0);
  lc.setRow(0,2,0b0);
  lc.setRow(0,3,0b0);
  lc.setRow(0,4,0b0);
  lc.setRow(0,5,0b0);
  lc.setRow(0,6,0b0);
  lc.setRow(0,7,0b0);

  // 4*7 segment init
  pinMode(seg_data,OUTPUT);
  pinMode(seg_lat,OUTPUT);
  pinMode(seg_clk,OUTPUT);

  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  pinMode(A3,INPUT);
  pinMode(A4,INPUT);
  pinMode(A5,INPUT);
}

void loop() {
  if(timer.isReady()){
    timer.reset();
    temp_ms();
    seg_ms();
    dot_ms();
    ir_ms();
    bt_ms();
    dot_mode=1;//==============================
  }
}


void temp_ms(){
  #define AVER 10
  static int temp_seqcnt=0,temp_t[AVER],temp_seq=0;
  long sum=0;
  temp_seqcnt++;
  if(temp_seqcnt>=100){ // ====================================500
    temp_seqcnt=0;
    waterlevel=analogRead(A0);    //water level senser
    if(waterlevel<350){
      waterlevel=0;
    } else {
      waterlevel=map(waterlevel,300,700,1,100);
    }
    
    temp_t[temp_seq++]=int(GetTemperature(ar(A1))*100);   //temp senser
    if(temp_seq>=AVER)temp_seq=0;
    for(int i=0;i<AVER;i++)sum+=temp_t[i];
    temp=int(sum/AVER);
    seg_dot=3;

    if(seg_mode==0)seg_num=temp;
    
 //   Serial.print("temp: ");  Serial.println(temp/100.0);    //debug
 //   Serial.print("Wat%: ");  Serial.println(waterlevel);

    //ir test ==============================================
//    static int irn=0;
//    ir_trg=1;
//    ir_data=irn;
//    irn=irn+1>=4?0:irn+1;
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
void ir_ms(){
  if(ir_trg){
    ir_trg=0;
    irsend.sendSony(ir_data,12);
  }
}
void bt_ms(){
  static int cnt=0;
  cnt++;
  if(cnt>=1000){
    cnt=0;
    //블루투스에 35도|30% 또는 100 (알람) 문자열 전송
    if(waterlevel>=90)bt_trg=1;
    if(bt_trg==0){  // 35도|30%
      char buf[20]={};
      sprintf(buf,"%d℃|%02d%%",int(temp/100),waterlevel);
      Serial.println(buf);
      btSerial.write(buf);
    }else{          // 100
      bt_trg=0;
      Serial.println("100");
      btSerial.write("100");
    }
  }
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
      Serial.print("    ");
      Serial.println(bt_time);
    }
}
